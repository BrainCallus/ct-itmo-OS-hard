#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include "entrypoint.h"
#include "http.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Churakova Alexandra");
MODULE_VERSION("0.01");
// 9d29d620-ec58-4f8a-b9e3-eaebde3555a8
char token[37];

void escape_url(char *str, char *result, int sz) {
  int j = 0;

  for (int i = 0; i < sz; i += 3) {
    result[i] = '%';
    sprintf(result + i + 1, "%02x", str[j++]);
  }
}

int catch_exception(int64_t res) {
  if (!res) return 0;
  
  else
    printk(KERN_ERR, "EXCEPTION");

  return 1;
}

struct file_system_type networkfs_fs_type = {
    .name = "networkfs",
    .mount = networkfs_mount,
    .kill_sb = networkfs_kill_sb,
};

struct inode_operations networkfs_inode_ops = {
    .lookup = networkfs_lookup,
    .create = networkfs_create,
    .unlink = networkfs_unlink,
    .mkdir = networkfs_mkdir,
    .rmdir = networkfs_rmdir,
    .link = networkfs_link,
};

struct file_operations networkfs_dir_ops = {
    .iterate = networkfs_iterate,
};

struct dentry* networkfs_mount(struct file_system_type* fs_type, int flags, const char* tkn, void* data) {
  struct dentry *ret;
  ret = mount_nodev(fs_type, flags, data, networkfs_fill_super);

  if (ret == NULL)
    printk(KERN_ERR "Can't mount file system");
  else
    printk(KERN_INFO "Mounted successfuly");

  for (int i = 0; i < 37; i++) token[i] = tkn[i];

  return ret;
}

int networkfs_fill_super(struct super_block* sb, void* data, int silent) {
  struct inode *inode;
  inode = networkfs_get_inode(NULL, sb, NULL, S_IFDIR, 1000);
  sb->s_root = d_make_root(inode);

  if (sb->s_root == NULL) return -ENOMEM;
  printk(KERN_INFO "return 0\n");
  return 0;
}

struct inode* networkfs_get_inode(struct user_namespace* ns, struct super_block* sb, const struct inode* dir, umode_t mode, int i_ino) {
  struct inode *inode;
  inode = new_inode(sb);
  inode->i_fop = &networkfs_dir_ops;
  inode->i_op = &networkfs_inode_ops;
  inode->i_ino = i_ino;
  inode->i_mode |= 777;

  if (inode != NULL) {
    inode_init_owner(&init_user_ns, inode, dir, mode);
  }
  return inode;
}

void networkfs_kill_sb(struct super_block* sb) {
  kfree(sb->s_fs_info);
  printk(KERN_INFO "Unmount success.\n");
}

int networkfs_iterate(struct file* filp, struct dir_context* ctx) {
  char fsname[10];
  struct dentry *dentry;
  struct inode *inode;
  unsigned long offset;
  unsigned char ftype;
  ino_t ino;
  ino_t cpy_ino;
  dentry = filp->f_path.dentry;
  inode = dentry->d_inode;
  offset = filp->f_pos;
  ino = inode->i_ino;
  struct entries response;
  char param_ino[11];
  snprintf(param_ino, 11, "%d", ino);
  int ret = networkfs_http_call(token, "list", (void *)&response, sizeof(struct entries), 1, "inode", param_ino);
  if (catch_exception(ret)) {
    return -1;
  }

  if (offset >= response.entries_count) {
    return 0;
  }

  strcpy(fsname, ".");
  ftype = DT_DIR;
  cpy_ino = ino;
  ctx->pos++;
  dir_emit(ctx, ".", 1, cpy_ino, ftype);

  strcpy(fsname, "..");
  cpy_ino = dentry->d_parent->d_inode->i_ino;
  ctx->pos++;
  dir_emit(ctx, "..", 2, cpy_ino, ftype);

  for (int i = 0; i < response.entries_count; i++) {
    printk("%s\n", response.entries[i].name);
    dir_emit(ctx, response.entries[i].name, strlen(response.entries[i].name), response.entries[i].ino, response.entries[i].entry_type);
  }
  ctx->pos += response.entries_count;

  return response.entries_count;
}



struct dentry* networkfs_lookup(struct inode* parent_inode, struct dentry* child_dentry, unsigned int flag) {
  ino_t root;
  struct inode *inode;
  const char *name = child_dentry->d_name.name;
  root = parent_inode->i_ino;
  struct entry_info info;
  char param_ino[11];
  snprintf(param_ino, 11, "%d", root);

  int length = strlen(name) * 3 + 1;
  char param_name[length];
  escape_url(name, param_name, length - 1);

  int ret = networkfs_http_call(token, "lookup", (void *)&info, sizeof(struct entry_info), 2, "parent", param_ino, "name", param_name);
  if (catch_exception(ret)) {
    return NULL;
  }

  inode = networkfs_get_inode(NULL, parent_inode->i_sb, NULL, info.entry_type == DT_DIR ? S_IFDIR : S_IFREG, info.ino);
  d_add(child_dentry, inode);
  return NULL;

}

int networkfs_create(struct user_namespace* ns, struct inode* parent_inode, struct dentry* child_dentry, umode_t mode, bool b) {
  ino_t root;
  struct inode *inode;
  const char *name = child_dentry->d_name.name;
  root = parent_inode->i_ino;
  ino_t resp = 0;
  char param_ino[11];
  snprintf(param_ino, 11, "%d", root);
  int length = strlen(name) * 3 + 1;
  char param_name[length];
  escape_url(name, param_name, length - 1);
  int ret = networkfs_http_call(token, "create", (ino_t)&resp, sizeof(ino_t), 3, "parent", param_ino, "name", param_name, "type", "file");

  if (catch_exception(ret)) {
    return -1;
  }
  inode = networkfs_get_inode(NULL, parent_inode->i_sb, NULL, S_IFREG | S_IRWXUGO, resp);
  inode->i_op = &networkfs_inode_ops;
  inode->i_fop = NULL;
  d_add(child_dentry, inode);
  return 0;
}

int networkfs_unlink(struct inode* parent_inode, struct dentry* child_dentry) {
  const char *name = child_dentry->d_name.name;
  ino_t root;
  root = parent_inode->i_ino;
  ino_t resp = 0;
  char param_ino[11];
  snprintf(param_ino, 11, "%d", root);
  int length = strlen(name) * 3 + 1;
  char param_name[length];
  escape_url(name, param_name, length - 1);

  int ret = networkfs_http_call(token, "unlink", (ino_t)&resp, sizeof(ino_t), 2, "parent", param_ino, "name", param_name);
  if (catch_exception(ret)) {
    return -1;
  }
  return 0;
}

int networkfs_mkdir(struct user_namespace* ns, struct inode* parent_inode, struct dentry* child_dentry, umode_t mode) {
  ino_t root;
  struct inode *inode;
  const char *name = child_dentry->d_name.name;
  root = parent_inode->i_ino;
  ino_t resp = 0;
  char param_ino[11];
  snprintf(param_ino, 11, "%d", root);
  int length = strlen(name) * 3 + 1;
  char param_name[length];
  escape_url(name, param_name, length - 1);

  int ret = networkfs_http_call(token, "create", (ino_t)&resp, sizeof(ino_t), 3, "parent", param_ino, "name", param_name, "type", "directory");
  if (catch_exception(ret)) {
    return -1;
  }
  inode = networkfs_get_inode(NULL, parent_inode->i_sb, NULL, S_IFDIR | S_IRWXUGO, resp);
  inode->i_op = &networkfs_inode_ops;
  inode->i_fop = NULL;
  d_add(child_dentry, inode);

  return 0;

}

int networkfs_rmdir(struct inode* parent_inode, struct dentry* child_dentry) {
  const char *name = child_dentry->d_name.name;
  ino_t root;
  root = parent_inode->i_ino;
  ino_t resp = 0;
  char param_ino[11];
  snprintf(param_ino, 11, "%d", root);
  int length = strlen(name) * 3 + 1;
  char param_name[length];
  escape_url(name, param_name, length - 1);

  int ret = networkfs_http_call(token, "rmdir", (ino_t)&resp, sizeof(ino_t), 2, "parent", param_ino, "name", param_name);
  if (catch_exception(ret)) {
    return -1;
  }
  return 0;
}

int networkfs_link(struct dentry* old_dentry, struct inode* parent_dir, struct dentry* new_dentry) {
  int resp = 0;
  const char parent_ino[11], source_ino[11];
  snprintf(parent_ino, 11, "%d", parent_dir->i_ino);
  snprintf(source_ino, 11, "%d", old_dentry->d_inode->i_ino);
  return networkfs_http_call(token, "link", (int)&resp, sizeof(int), 3, "source", source_ino, "parent", parent_ino, "name", new_dentry->d_name.name);
}

int networkfs_init(void) {
  printk(KERN_INFO "INIT WORK\n");
  register_filesystem(&networkfs_fs_type);
  return 0;
}

void networkfs_exit(void) {
  printk(KERN_INFO "FINISH WORK\n");
  unregister_filesystem(&networkfs_fs_type);
}

module_init(networkfs_init);
module_exit(networkfs_exit);
