#FIXME: cache_v7 and cache-cp15 will go away eventually

bootblock-y += syslib.c
bootblock-$(CONFIG_EARLY_CONSOLE) += early_console.c
bootblock-y += cache.c
bootblock-y += cache_v7.c
bootblock-y += cache-cp15.c

romstage-y += cache.c
romstage-y += cache_v7.c
romstage-y += cache-cp15.c
romstage-y += div0.c
romstage-y += syslib.c
romstage-$(CONFIG_EARLY_CONSOLE) += early_console.c

ramstage-y += div0.c
#ramstage-y += interrupts.c
#ramstage-y += memcpy.S
#ramstage-y += memset.S
ramstage-y += syslib.c
ramstage-y += cache.c
ramstage-y += cache_v7.c
ramstage-y += cache-cp15.c

#FIXME(dhendrix): should this be a config option?
romstage-y += eabi_compat.c
ramstage-y += eabi_compat.c
bootblock-y += eabi_compat.c
