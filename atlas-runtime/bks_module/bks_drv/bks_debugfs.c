#include <linux/cdev.h>
#include <linux/debugfs.h>
#include <linux/list.h>
#include <linux/mm.h>

#include "bks.h"

bool bks_ctl_flags[NUM_BKS_CTL_FLAGS];
const char *bks_ctl_flag_names[NUM_BKS_CTL_FLAGS] = {
    "fake_psf_manager",
    "direct_swap",
};

int bks_debugfs_init(void) {
    struct dentry *parent = debugfs_lookup("hermit", NULL);
    struct dentry *root;
    int i;
    if (!parent) {
        pr_err("Failed to lookup parent debugfs directory");
        return -1;
    }
    root = debugfs_create_dir("bks", parent);
    if (!root) {
        pr_err("Failed to create bks debugfs directory");
        return -1;
    }

    for (i = 0; i < NUM_BKS_CTL_FLAGS; i++)
        bks_ctl_flags[i] = false;

    for (i = 0; i < NUM_BKS_CTL_FLAGS; i++)
        debugfs_create_bool(bks_ctl_flag_names[i], 0666, root,
                            &bks_ctl_flags[i]);
    return 0;
}

void bks_debugfs_exit(void) {
    struct dentry *parent = debugfs_lookup("hermit", NULL);
    struct dentry *root;
    if (!parent)
        return;
    root = debugfs_lookup("bks", parent);
    if (!root)
        return;
    debugfs_remove_recursive(root);
}