# MSM Machine Support

snd-soc-hostless-pcm-objs := msm-pcm-hostless.o
obj-$(CONFIG_SND_SOC_MSM_HOSTLESS_PCM) += snd-soc-hostless-pcm.o

obj-$(CONFIG_SND_SOC_MSM_QDSP6V2_INTF) += qdsp6v2/

snd-soc-qdsp6v2-objs := msm-dai-fe.o
obj-$(CONFIG_SND_SOC_QDSP6V2) += snd-soc-qdsp6v2.o

#for APQ8084 sound card driver
snd-soc-apq8084-objs := apq8084.o
obj-$(CONFIG_SND_SOC_APQ8084) += snd-soc-apq8084.o

#for CPE drivers
snd-soc-cpe-objs := msm-cpe-lsm.o
obj-$(CONFIG_SND_SOC_CPE) += snd-soc-cpe.o

# for MSM 8994 sound card driver
snd-soc-msm8994-objs := msm8994.o
obj-$(CONFIG_SND_SOC_MSM8994) += snd-soc-msm8994.o

# for MSM8996 sound card driver
snd-soc-msm8996-objs := msm8996.o apq8096-auto.o apq8096-i2c.o
obj-$(CONFIG_SND_SOC_MSM8996) += snd-soc-msm8996.o

# for MSM 8x16/8952 sound card driver
snd-soc-msm8x16-objs := msm8952.o msm-audio-pinctrl.o
snd-soc-msm8x16-objs += msm8952-slimbus.o msm8952-dai-links.o
#ifdef VENDOR_EDIT
#Jianfeng.Qiu@Swdp.Multimedia  2016/08/10, Add for oppo audio custom
snd-soc-msm8x16-objs += oppo_audio_custom.o
#endif /* VENDOR_EDIT */

#ifdef VENDOR_EDIT
#John.Xu 2016-02-05 add for tfa9890 and ak4375 multi-project baseline
ifneq ($(filter $(OPPO_TARGET_DEVICE),MSM_16017 MSM_16027),)
KBUILD_CFLAGS += -DOPPO_QUIN_MI2S_AK4375
KBUILD_CFLAGS += -DOPPO_QUAT_MI2S_TFA98XX
endif
#endif /* VENDOR_EDIT */
obj-$(CONFIG_SND_SOC_MSM8X16) += snd-soc-msm8x16.o

# for MDM Californium sound card driver
snd-soc-mdmcalifornium-objs := mdmcalifornium.o
obj-$(CONFIG_SND_SOC_MDMCALIFORNIUM) += snd-soc-mdmcalifornium.o

# for MDM 9607 sound card driver
snd-soc-mdm9607-objs := mdm9607.o
obj-$(CONFIG_SND_SOC_MDM9607) += snd-soc-mdm9607.o

# for MSM 8909 sound card driver
snd-soc-msm8909-objs := msm8952.o msm-audio-pinctrl.o
obj-$(CONFIG_SND_SOC_MSM8909) += snd-soc-msm8909.o

# for MDM9640 sound card driver
snd-soc-mdm9640-objs := mdm9640.o
obj-$(CONFIG_SND_SOC_MDM9640) += snd-soc-mdm9640.o