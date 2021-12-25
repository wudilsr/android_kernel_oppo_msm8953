/**
 * Copyright 2008-2013 OPPO Mobile Comm Corp., Ltd, All rights reserved.
 * VENDOR_EDIT:
 * FileName:devinfo.c
 * ModuleName:devinfo
 * Author: wangjc
 * Create Date: 2013-10-23
 * Description:add interface to get device information.
 * History:
   <version >  <time>  <author>  <desc>
   1.0		2013-10-23	wangjc	init
   2.0      2015-04-13  hantong modify as platform device  to support diffrent configure in dts
*/

#include <linux/module.h>
#include <linux/proc_fs.h>
#include <soc/qcom/smem.h>
#include <soc/oppo/device_info.h>
#include <soc/oppo/oppo_project.h>
#include <linux/slab.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include "../../../fs/proc/internal.h"
#include <asm/uaccess.h>

#define DEVINFO_NAME "devinfo"
#define INFO_BUF_LEN 64

static int mainboard_res = 0;

static struct of_device_id devinfo_id[] = {
	{.compatible = "oppo-devinfo",},
	{},
};

struct devinfo_data { 
	struct platform_device *devinfo;
	int hw_id1_gpio;
	int hw_id2_gpio;
	int hw_id3_gpio;
	int sub_hw_id1;
	int sub_hw_id2;
	int audio_hw_id1;
	int ant_select_gpio;
};

static struct proc_dir_entry *parent = NULL;

static void *device_seq_start(struct seq_file *s, loff_t *pos)
{
	static unsigned long counter = 0;
	if (*pos == 0) {
		return &counter;
	} else {
		*pos = 0; 
		return NULL;
	}
}

static void *device_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	return NULL;
}

static void device_seq_stop(struct seq_file *s, void *v)
{
	return;
}

static int device_seq_show(struct seq_file *s, void *v)
{
	struct proc_dir_entry *pde = s->private;
	struct manufacture_info *info = pde->data;

	if (info)
		seq_printf(s, "Device version:\t\t%s\n"
				  "Device manufacture:\t\t%s\n",
				  info->version,
				  info->manufacture);
	return 0;
}

static struct seq_operations device_seq_ops = {
	.start = device_seq_start,
	.next = device_seq_next,
	.stop = device_seq_stop,
	.show = device_seq_show
};

static int device_proc_open(struct inode *inode,struct file *file)
{
	int ret = seq_open(file,&device_seq_ops);

	if (!ret) {
		struct seq_file *sf = file->private_data;
		sf->private = PDE(inode);
	}

	return ret;
}

static const struct file_operations device_node_fops = {
	.read =  seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
	.open = device_proc_open,
	.owner = THIS_MODULE,
};

int register_device_proc(char *name, char *version, char *manufacture)
{
	struct proc_dir_entry *d_entry;
	struct manufacture_info *info;

	if (!parent) {
		parent = proc_mkdir ("devinfo", NULL);
		if (!parent) {
			pr_err("can't create devinfo proc\n");
			return -ENOENT;
		}
	}

	info = kzalloc(sizeof *info, GFP_KERNEL);
	info->version = version;
	info->manufacture = manufacture;
	d_entry = proc_create_data(name, S_IRUGO, parent, &device_node_fops,
			info);

	if (!d_entry) {
		pr_err("create %s proc failed.\n", name);
		kfree(info);
		return -ENOENT;
	}

	return 0;
}

static void dram_type_add(void)
{
	struct manufacture_info dram_info;
	int *p = NULL;

	p = (int *)smem_alloc(SMEM_ID_VENDOR2, 4, 0, 0);

	if (p) {
		switch (*p) {
			case 0x01:
				dram_info.version = "K3QF4F40BM-FGCF FBGA";
				dram_info.manufacture = "SAMSUNG";
				break;
			case 0x06:
				dram_info.version = "H9CKNNNCPTMRPR FBGA";
				dram_info.manufacture = "HYNIX";
				break;
			default:
				dram_info.version = "unknown";
				dram_info.manufacture = "unknown";
		}
	} else {
		dram_info.version = "unknown";
		dram_info.manufacture = "unknown";
	}

	register_device_proc("ddr", dram_info.version, dram_info.manufacture);
}

static int get_ant_select_gpio(struct devinfo_data *devinfo_data)
{
	int ret;
	struct device_node *np;	
	pr_err("srd get_ant_select_gpio\n");

	if (!devinfo_data) {
		pr_err("devinfo_data is NULL\n");
		return 0;
	}

	np = devinfo_data->devinfo->dev.of_node;

	devinfo_data->hw_id1_gpio = of_get_named_gpio(
			np, "Hw,ant_select-gpio", 0);
	if (devinfo_data->ant_select_gpio < 0) {
		if (gpio_is_valid(devinfo_data->ant_select_gpio)) {
			ret = gpio_request(devinfo_data->ant_select_gpio, "ant_select-gpio");
				if (ret) {
					pr_err("unable to request gpio [%d]\n", devinfo_data->ant_select_gpio);
				}
		}
	} else {
		pr_err("devinfo_data->ant_select_gpio not specified\n");
	}

	return ret;
}

static int get_hw_opreator_version(struct devinfo_data *devinfo_data)
{
	int hw_operator_name = 0;
	int ret;
	int id1 = -1;
	int id2 = -1;
	int id3 = -1;
	struct device_node *np;	

	if (!devinfo_data) {
		pr_err("devinfo_data is NULL\n");
		return 0;
	}

	np = devinfo_data->devinfo->dev.of_node;

	devinfo_data->hw_id1_gpio = of_get_named_gpio(
			np, "Hw,operator-gpio1", 0);
	if (devinfo_data->hw_id1_gpio < 0)
		pr_err("devinfo_data->hw_id1_gpio not specified\n");

	devinfo_data->hw_id2_gpio = of_get_named_gpio(
			np, "Hw,operator-gpio2", 0);
	if (devinfo_data->hw_id2_gpio < 0)
		pr_err("devinfo_data->hw_id2_gpio not specified\n");

	devinfo_data->hw_id3_gpio = of_get_named_gpio(
			np, "Hw,operator-gpio3", 0);
	if (devinfo_data->hw_id3_gpio < 0)
		pr_err("devinfo_data->hw_id3_gpio not specified\n");

	if (devinfo_data->hw_id1_gpio >= 0) {
		ret = gpio_request(devinfo_data->hw_id1_gpio,"HW_ID1");
		if (ret)
			pr_err("unable to request gpio [%d]\n",devinfo_data->hw_id1_gpio);
		else
			id1 = gpio_get_value(devinfo_data->hw_id1_gpio);	
 	}

 	if (devinfo_data->hw_id2_gpio >= 0) {
		ret = gpio_request(devinfo_data->hw_id2_gpio,"HW_ID2");
		if (ret)
			pr_err("unable to request gpio [%d]\n",devinfo_data->hw_id2_gpio);
		else
			id2 = gpio_get_value(devinfo_data->hw_id2_gpio);	
 	}

 	if (devinfo_data->hw_id3_gpio >= 0) {
		ret = gpio_request(devinfo_data->hw_id3_gpio,"HW_ID3");
		if (ret)
			pr_err("unable to request gpio [%d]\n",devinfo_data->hw_id3_gpio);
		else
			id3 = gpio_get_value(devinfo_data->hw_id3_gpio);
 	}
	if (is_project(OPPO_16017)) {
		if ((id1==1) && (id2==1) && (id3==1))
			hw_operator_name = OPERATOR_ALL_CHINA_CARRIER;
		else if ((id1==0) && (id2==1) && (id3==1))
			hw_operator_name = OPERATOR_CHINA_MOBILE;
		else if ((id1==0) && (id2==1) && (id3==0))
			mainboard_res = MAINBOARD_RESOURCE2;
		else
			hw_operator_name = OPERATOR_UNKOWN;
	}
	pr_info("hw_operator_name [%d]\n",hw_operator_name);
	return hw_operator_name;
}

static void sub_mainboard_verify(struct devinfo_data *devinfo_data)
{
	int ret;
	int id1 = -1;
	static char temp_manufacture_sub[INFO_BUF_LEN] = {0};
	struct device_node *np;
	struct manufacture_info mainboard_info;

	if (!devinfo_data) {
		pr_err("devinfo_data is NULL\n");
		return;
	}

	np = devinfo_data->devinfo->dev.of_node;

	devinfo_data->sub_hw_id1 = of_get_named_gpio(np, "Hw,sub_hwid_1", 0);
	if (devinfo_data->sub_hw_id1 < 0)
		pr_err("devinfo_data->sub_hw_id1 not specified\n");

	if (devinfo_data->sub_hw_id1 >= 0) {
		ret = gpio_request(devinfo_data->sub_hw_id1, "SUB_HW_ID1");
		if (ret)
			pr_err("unable to request gpio [%d]\n", devinfo_data->sub_hw_id1);
		else
			id1 = gpio_get_value(devinfo_data->sub_hw_id1);	
 	}

	mainboard_info.manufacture = temp_manufacture_sub;
	mainboard_info.version = "Qcom";

	switch (get_project()) {
		case OPPO_16017: case OPPO_16027: {
			if (id1 == 1)
				snprintf(mainboard_info.manufacture, INFO_BUF_LEN, "%d-sub-china", get_project());
			else
				mainboard_info.manufacture = "sub-UNSPECIFIED";
			break;
		}
		case OPPO_16061: {
			if (id1 == 1)
				snprintf(mainboard_info.manufacture, INFO_BUF_LEN, "%d-sub-china", get_project());
			else
				snprintf(mainboard_info.manufacture, INFO_BUF_LEN, "%d-sub-oversea", get_project());
			break;
		}
		default: {
			snprintf(mainboard_info.manufacture, INFO_BUF_LEN,"%d-%d", get_project(), get_Operator_Version());
			break;
		}
	}
	register_device_proc("sub_mainboard", mainboard_info.version, mainboard_info.manufacture);
}

static void audio_board_verify(struct devinfo_data *devinfo_data)
{
	int ret;
	int id1 = -1;
	static char temp_manufacture_audio[INFO_BUF_LEN] = {0};
	struct device_node *np;	
	struct manufacture_info mainboard_info;

	if (!devinfo_data) {
		pr_err("devinfo_data is NULL\n");
		return;
	}

	np = devinfo_data->devinfo->dev.of_node;

	devinfo_data->audio_hw_id1 = of_get_named_gpio(np, "Hw,audio_hwid_1", 0);
	if (devinfo_data->audio_hw_id1 < 0)
		pr_err("devinfo_data->sub_hw_id1 not specified\n");

	if (devinfo_data->audio_hw_id1 >= 0 ) {
		ret = gpio_request(devinfo_data->audio_hw_id1, "audio_hw_id1");
		if (ret)
			pr_err("unable to request gpio [%d]\n", devinfo_data->audio_hw_id1);
		else
			id1 = gpio_get_value(devinfo_data->audio_hw_id1);	
 	}

	mainboard_info.manufacture = temp_manufacture_audio;
	mainboard_info.version = "Qcom";

	switch (get_project()) {
		case OPPO_16017: case OPPO_16027: {
			snprintf(mainboard_info.manufacture, INFO_BUF_LEN, "%d-audio-china", get_project());
			break;
		}
		case OPPO_16061: {
			if (id1 == 0)
				snprintf(mainboard_info.manufacture, INFO_BUF_LEN, "%d-audio-china", get_project());
			else
				snprintf(mainboard_info.manufacture, INFO_BUF_LEN, "%d-audio-oversea", get_project());
			break;
		}
		default: {
			snprintf(mainboard_info.manufacture, INFO_BUF_LEN, "%d-%d", get_project(), get_Operator_Version());
			break;
		}
	}
	register_device_proc("audio_mainboard", mainboard_info.version, mainboard_info.manufacture);
}

static void mainboard_verify(struct devinfo_data *devinfo_data)
{
	struct manufacture_info mainboard_info;
	int hw_opreator_version = 0;
	static char temp_manufacture[INFO_BUF_LEN] = {0};

	if (!devinfo_data) {
		pr_err("devinfo_data is NULL\n");
		return;
	}

	hw_opreator_version = get_hw_opreator_version(devinfo_data);
	mainboard_info.manufacture = temp_manufacture;

	switch (get_PCB_Version()) {
		case HW_VERSION__10:
			mainboard_info.version = "10";
			snprintf(mainboard_info.manufacture, INFO_BUF_LEN, "%d-SA", hw_opreator_version);
			break;
		case HW_VERSION__11:
			mainboard_info.version = "11";
			snprintf(mainboard_info.manufacture, INFO_BUF_LEN, "%d-SB", hw_opreator_version);
			break;
		case HW_VERSION__12:
			mainboard_info.version = "12";
			snprintf(mainboard_info.manufacture, INFO_BUF_LEN, "%d-SC", hw_opreator_version);
			break;
		case HW_VERSION__13:
			mainboard_info.version = "13";
			snprintf(mainboard_info.manufacture, INFO_BUF_LEN, "%d-SD", hw_opreator_version);
			break;
		case HW_VERSION__14:
			mainboard_info.version = "14";
			snprintf(mainboard_info.manufacture, INFO_BUF_LEN, "%d-SE", hw_opreator_version);
			break;
		case HW_VERSION__15:
			mainboard_info.version = "15";
			snprintf(mainboard_info.manufacture, INFO_BUF_LEN, "%d-(T3-T4)" ,hw_opreator_version);
			break;
		default:	
			mainboard_info.version = "UNKOWN";
			snprintf(mainboard_info.manufacture, INFO_BUF_LEN, "%d-UNKOWN", hw_opreator_version);
	}	
	register_device_proc("mainboard", mainboard_info.version, mainboard_info.manufacture);	
}

static void pa_verify(void)
{
	struct manufacture_info pa_info;

	switch (get_Modem_Version()) {
		case 0:
			pa_info.version = "0";
			pa_info.manufacture = "RFMD PA";
			break;
		case 1:	
			pa_info.version = "1";
			pa_info.manufacture = "SKY PA";
			break;
		case 3:
			pa_info.version = "3";
			pa_info.manufacture = "AVAGO PA";
			break;
		default:
			pa_info.version = "UNKOWN";
			pa_info.manufacture = "UNKOWN";
	}

	register_device_proc("pa", pa_info.version, pa_info.manufacture);

}

static ssize_t mainboard_resource_read_proc(struct file *file, char __user *buf, 
		size_t count, loff_t *off)
{
	char page[256] = {0};
	int len = 0;
	len = sprintf(page,"%d",mainboard_res);

	if (len > *off)
		len -= *off;
	else
		len = 0;

	if (copy_to_user(buf,page,(len < count ? len : count))) {
		return -EFAULT;
	}

	*off += len < count ? len : count;
	return (len < count ? len : count);
}

struct file_operations mainboard_res_proc_fops = {
	.read = mainboard_resource_read_proc,
	.write = NULL,
};


static int devinfo_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct devinfo_data *devinfo_data = NULL;
	struct proc_dir_entry *pentry;

	devinfo_data = kzalloc(sizeof(struct devinfo_data), GFP_KERNEL);

	if (devinfo_data == NULL) {
		pr_err("devinfo_data kzalloc failed\n");
		ret = -ENOMEM;
		return ret;
	}

	devinfo_data->devinfo = pdev;

	get_ant_select_gpio(devinfo_data);
	mainboard_verify(devinfo_data);
	sub_mainboard_verify(devinfo_data);
	audio_board_verify(devinfo_data);
	dram_type_add();

	pentry = proc_create("mainboard", S_IRUGO, NULL, &mainboard_res_proc_fops);
	if (!pentry)
		pr_err("create prjVersion proc failed.\n");

	return ret;

	if (!parent) {
		parent = proc_mkdir ("devinfo", NULL);
		if (!parent) {
			pr_err("can't create devinfo proc\n");
			ret = -ENOENT;
		}
	}

	pa_verify();
}

static int devinfo_remove(struct platform_device *dev)
{
	remove_proc_entry(DEVINFO_NAME, NULL);
	return 0;
}

static struct platform_driver devinfo_platform_driver = {
	.probe = devinfo_probe,
	.remove = devinfo_remove,
	.driver = {
		.name = DEVINFO_NAME,
		.of_match_table = devinfo_id,
	},
};

module_platform_driver(devinfo_platform_driver);

MODULE_DESCRIPTION("OPPO device info");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Wangjc <wjc@oppo.com>");
