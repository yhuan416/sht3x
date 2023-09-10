#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of_gpio.h>
#include <linux/semaphore.h>
#include <linux/timer.h>
#include <linux/i2c.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define SHT3X_NAME  "sht3x"

#define SHT3X_CMD_LENGTH 2

struct sht3x_dev
{
    struct i2c_client *client;
    dev_t devid;
    struct cdev cdev;
    struct class *class;
    struct device *device;
    struct device_node *nd;
};

static int sht3x_open(struct inode *inode, struct file *filp)
{
    return 0;
}

static ssize_t sht3x_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
    return 0;
}

static ssize_t sht3x_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
    return 0;
}

static int sht3x_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static struct file_operations sht3x_fops =
{
    .owner = THIS_MODULE,
    .open = sht3x_open,
    .read = sht3x_read,
    .write = sht3x_write,
    .release = sht3x_release,
};

static int sht3x_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int ret = 0;
    struct sht3x_dev *sht3xdev = NULL;

    printk("sht3x probe.\r\n");

    sht3xdev = devm_kzalloc(&client->dev, sizeof(*sht3xdev), GFP_KERNEL);
    if (sht3xdev == NULL)
    {
        printk("devm_kzalloc failed.\r\n");
        return -ENOMEM;
    }

    // alloc dev_id
    ret = alloc_chrdev_region(&sht3xdev->devid, 0, 1, SHT3X_NAME);
    if (ret < 0)
    {
        pr_err("%s Couldn't alloc_chrdev_region, ret=%d\r\n", SHT3X_NAME, ret);
        return -ENOMEM;
    }

    // init cdev
    sht3xdev->cdev.owner = THIS_MODULE;
    cdev_init(&sht3xdev->cdev, &sht3x_fops);

    // add cdev
    ret = cdev_add(&sht3xdev->cdev, sht3xdev->devid, 1);
    if (ret < 0)
    {
        pr_err("%s cdev_add failed, ret=%d\r\n", SHT3X_NAME, ret);
        goto cdev_add_failed;
    }

    // create class
    sht3xdev->class = class_create(THIS_MODULE, SHT3X_NAME);
    if (IS_ERR(sht3xdev->class))
    {
        pr_err("%s class_create failed\r\n", SHT3X_NAME);
        ret = PTR_ERR(sht3xdev->class);
        goto class_create_failed;
    }

    // create device
    sht3xdev->device = device_create(sht3xdev->class, NULL, sht3xdev->devid, sht3xdev, SHT3X_NAME);
    if (IS_ERR(sht3xdev->device))
    {
        pr_err("%s device_create failed\r\n", SHT3X_NAME);
        ret = PTR_ERR(sht3xdev->device);
        goto device_create_failed;
    }

    sht3xdev->client = client;
    i2c_set_clientdata(client, sht3xdev);

    return 0;

device_create_failed:
    class_destroy(sht3xdev->class);

class_create_failed:
    cdev_del(&sht3xdev->cdev);

cdev_add_failed:
    unregister_chrdev_region(sht3xdev->devid, 1);

    return ret;
}

static int sht3x_remove(struct i2c_client *client)
{
    struct sht3x_dev *sht3xdev = i2c_get_clientdata(client);

    printk("sht3x remove.\r\n");

    // delete device
    device_destroy(sht3xdev->class, sht3xdev->devid);

    // delete class
    class_destroy(sht3xdev->class);

    // delete cdev
    cdev_del(&sht3xdev->cdev);

    // unregister dev_id
    unregister_chrdev_region(sht3xdev->devid, 1);

    return 0;
}

/* 传统匹配方式 ID 列表 */
static const struct i2c_device_id sht3x_id[] = {
    {"sensirion,sht3x", 0},
    { }
};

/* 设备树匹配列表 */
static const struct of_device_id sht3x_of_match[] = {
    { .compatible = "sensirion,sht3x" },
    { }
};

/* i2c 驱动结构体 */
static struct i2c_driver sht3x_driver = {
    .probe = sht3x_probe,
    .remove = sht3x_remove,
    .driver = {
        .owner = THIS_MODULE,
        .name = "sht3x",
        .of_match_table = sht3x_of_match,
    },
    .id_table = sht3x_id,
};

module_i2c_driver(sht3x_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("yhuan416 <yhuan416@foxmail.com>");
