#pragma once

struct entry_info {
  unsigned char entry_type;
  ino_t ino;
};

struct entry {
  unsigned char entry_type;
  ino_t ino;
  char name[256];
};

struct entries {
  size_t entries_count;
  struct entry entries[16];
};

struct content {
  u64 content_length;
  char content[];
};

void escape_url(char *, char *, int);
struct dentry *networkfs_mount(struct file_system_type *fs, int a, const char *b, void *c);
void networkfs_kill_sb(struct super_block *sb);
int networkfs_fill_super(struct super_block *sb, void *data, int silent);
struct inode *networkfs_get_inode(struct user_namespace *ns, struct super_block *sb, const struct inode *i, umode_t m, int a);
struct dentry *networkfs_lookup(struct inode *, struct dentry *, unsigned int);
int networkfs_create(struct user_namespace *, struct inode *, struct dentry *, umode_t mode, bool b);
int networkfs_unlink(struct inode *, struct dentry *);
int networkfs_mkdir(struct user_namespace *, struct inode *, struct dentry *, umode_t);
int networkfs_rmdir(struct inode *, struct dentry *);
int networkfs_link(struct dentry *, struct inode *, struct dentry *);
int networkfs_iterate(struct file *, struct dir_context *);
