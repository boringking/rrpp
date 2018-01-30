#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

#include "ip1829.h"
#include "ip1829reg.h"
#include "ip1829ds.h"
#include "ip1829op.h"
#include "ip218.h"

#define	ALL_PHY_PORTS_LIST	(~(-1 << (SWITCH_MAX_IPORT_CNT-1)))
#define MAX_PHY_TP_NUM	24

#define IP1829DRV_REG_DEBUG
//#define IP1829DEBUG
#ifdef	IP1829DEBUG
#define	ip1829drv_dbg(fmt, ...)	printk(KERN_ALERT "<%s:%d>" fmt, __func__, __LINE__, ##__VA_ARGS__)
#else
#define	ip1829drv_dbg(...)
#endif//IP1829DEBUG

#define FUNC_MSG_IN		ip1829drv_dbg("IN \n")
#define FUNC_MSG_OUT	ip1829drv_dbg("OUT\n")

extern u8 CPU_IF_SPEED_NORMAL;

#ifdef TEST_REG
u16 RegList[16][256];
u8 pg;
#endif//TEST_REG

#define ip1829delay()		(udelay(100))

/*=============   misc. driver functions   ===================*/

//#define ACCESS_REG_BY_MDIO
#ifdef ACCESS_REG_BY_MDIO
//--------------  p1829 project GPIO MDIO Function ---------------------
#define MDIO_DELAY	0
#define MDIO_RD		0
#define MDIO_WR		1
#define GPIO_MDIO_OFFSET 0x0
#define GPIO_MDC_OFFSET 0x1
#include <asm/io.h>
static DEFINE_MUTEX(ip1829_gpio_lock);
#define IP1829_GPIO_BASE      0xBEE00000
#define IP1829_GPIO_OUTPUT_VALUE_0_OFFSET	0x0018
#define IP1829_GPIO_OUTPUT_ENABLE_0_OFFSET	0x0020
#define IP1829_GPIO_DIRECTION_0_OFFSET		0x0008
#define IP1829_GPIO_INPUT_VALUE_0_OFFSET	0x0010
#define IP1829_GPIO_SET_VALUE_1 0x1
#define IP1829_GPIO_SET_VALUE_0 0x0
static int ip1829_gpio_get(unsigned offset)
{
	void __iomem				*base;
	void __iomem				*base_offset = 0x0000;
	u32					val;

	mutex_lock(&ip1829_gpio_lock);

	if((0<=offset)&&(offset<=31))
		base_offset = 0x0000;
	else if((32<=offset)&&(offset<=47)) {
		base_offset = (void __iomem *)0x0004;
		offset -= 32;
	}

	base = IP1829_GPIO_BASE + base_offset + IP1829_GPIO_INPUT_VALUE_0_OFFSET;
	val = readl(base);
	val = ((val & BIT(offset))>>offset);

	mutex_unlock(&ip1829_gpio_lock);

	return val;
}
static void ip1829_gpio_set(unsigned offset,int value)
{
	void __iomem				*base;
	void __iomem				*base_offset = 0x0000;
	u32					val;

	mutex_lock(&ip1829_gpio_lock);

	if((0<=offset)&&(offset<=31))
		base_offset = 0x0000;
	else if((32<=offset)&&(offset<=47)) {
		base_offset = (void __iomem *)0x0004;
		offset -= 32;
	}

	base = IP1829_GPIO_BASE + base_offset + IP1829_GPIO_OUTPUT_VALUE_0_OFFSET;
	val = readl(base);
	val = (value == IP1829_GPIO_SET_VALUE_1) ? (val | BIT(offset)) : (val & ~((BIT(offset))));
	writel(val, base);

	mutex_unlock(&ip1829_gpio_lock);

	return;
}
static int ip1829_gpio_direction_out(unsigned offset,int value)
{
	void __iomem				*base;
	void __iomem				*base_offset = 0x0000;
	u32					val;

	mutex_lock(&ip1829_gpio_lock);

	if((0<=offset)&&(offset<=31))
		base_offset = 0x0000;
	else if((32<=offset)&&(offset<=47)) {
		base_offset = (void __iomem *)0x0004;
		offset -= 32;
	}

	base = IP1829_GPIO_BASE + base_offset + IP1829_GPIO_DIRECTION_0_OFFSET;
	val = readl(base);
	val |= BIT(offset);
	writel(val, base);

	base = IP1829_GPIO_BASE + base_offset + IP1829_GPIO_OUTPUT_VALUE_0_OFFSET;
	val = readl(base);
	val = (value == IP1829_GPIO_SET_VALUE_1) ? (val | BIT(offset)) : (val & ~((BIT(offset))));
	writel(val, base);

	base = IP1829_GPIO_BASE + base_offset + IP1829_GPIO_OUTPUT_ENABLE_0_OFFSET;
	val = readl(base);
	val |= BIT(offset);
	writel(val, base);

	mutex_unlock(&ip1829_gpio_lock);

	return 0;
}
static int ip1829_gpio_direction_in( unsigned offset)
{
	void __iomem				*base;
	void __iomem				*base_offset = 0x0000;
	u32					val;

	mutex_lock(&ip1829_gpio_lock);

	if((0<=offset)&&(offset<=31))
		base_offset = 0x0000;
	else if((32<=offset)&&(offset<=47)) {
		base_offset = (void __iomem *)0x0004;
		offset -= 32;
	}

	base = IP1829_GPIO_BASE + base_offset + IP1829_GPIO_OUTPUT_ENABLE_0_OFFSET;
	val = readl(base);
	val &= ~(BIT(offset));
	writel(val, base);

	base = IP1829_GPIO_BASE + base_offset + IP1829_GPIO_DIRECTION_0_OFFSET;
	val = readl(base);
	val &= ~(BIT(offset));
	writel(val, base);

	base = IP1829_GPIO_BASE + base_offset + IP1829_GPIO_INPUT_VALUE_0_OFFSET;
	val = readl(base);
	val = ((val & BIT(offset))>>offset);

	mutex_unlock(&ip1829_gpio_lock);

	return val;
}
void mdio_1(void){

	int i;
	//set MDIO to 1
	//set MDC to 0
	ip1829_gpio_set(GPIO_MDIO_OFFSET,1);
	ip1829_gpio_set(GPIO_MDC_OFFSET,0);

	for(i=0;i<MDIO_DELAY;i++);

	//set MDIO to 1
	//set MDC to 1
	ip1829_gpio_set(GPIO_MDIO_OFFSET,1);
	ip1829_gpio_set(GPIO_MDC_OFFSET,1);

	for(i=0;i<MDIO_DELAY;i++);
}

void mdio_0(void){

	int i;
	//set MDIO to 0
	//set MDC to 0
	ip1829_gpio_set(GPIO_MDIO_OFFSET,0);
	ip1829_gpio_set(GPIO_MDC_OFFSET,0);

	for(i=0;i<MDIO_DELAY;i++);

	//set MDIO to 0
	//set MDC to 1
	ip1829_gpio_set(GPIO_MDIO_OFFSET,0);
	ip1829_gpio_set(GPIO_MDC_OFFSET,1);

	for(i=0;i<MDIO_DELAY;i++);
}

void mdio_z(void){

	int i;
	//set MDC to 0
	ip1829_gpio_set(GPIO_MDC_OFFSET,0);

	for(i=0;i<MDIO_DELAY;i++);
	//set MDC to 1
	ip1829_gpio_set(GPIO_MDC_OFFSET,1);

	for(i=0;i<MDIO_DELAY;i++);
}

void mdio_start(void){
	mdio_0();
	mdio_1();
}

void mdio_rw(int rw){
	if(rw==MDIO_RD){
		mdio_1();
		mdio_0();
	}else{
		mdio_0();
		mdio_1();
	}
}
void mdio_set_MDC_MDIO_direction(unsigned char mdc, unsigned char mdio)//0:input, 1:output for mdc/mdio values
{
	if(mdc) ;
	if(mdio)
		ip1829_gpio_direction_out(GPIO_MDIO_OFFSET,0);
	else
		ip1829_gpio_direction_in(GPIO_MDIO_OFFSET);
}
void ic_mdio_wr(unsigned short pa, unsigned short ra, unsigned short va){
	int i=0;
	unsigned short data=0;

	//set MDC/MDIO pins to GPIO mode
	ip1829_gpio_direction_out(GPIO_MDC_OFFSET,0);
	ip1829_gpio_direction_out(GPIO_MDIO_OFFSET,0);

	//set MDC direction to output
	//set MDIO direction to output
	mdio_set_MDC_MDIO_direction(1,1);

	for(i=0;i<32;i++)
		mdio_1();
	mdio_start();
	mdio_rw(MDIO_WR);
	for(i=0;i<5;i++){
		if((pa>>(5-1-i))%2)
			mdio_1();
		else
			mdio_0();
	}
	for(i=0;i<5;i++){
		if((ra>>(5-1-i))%2)
			mdio_1();
		else
			mdio_0();
	}
	mdio_1();
	mdio_0();
	for(i=0;i<16;i++){
		data=va<<i;
		data=data>>15;
		if(data==1)
			mdio_1();
		else
			mdio_0();
	}
}
unsigned short ic_mdio_rd(unsigned short pa, unsigned short ra){
	int i=0,j=0;
	unsigned short data=0;
	int regBit;
	unsigned char debug[16];

	//set MDC/MDIO pins to GPIO mode
	ip1829_gpio_direction_out(GPIO_MDC_OFFSET,0);
	ip1829_gpio_direction_out(GPIO_MDIO_OFFSET,0);

	//set MDC/MDIO PIN direction
	//mdio_set_MDC_MDIO_dir();
	//MDC direction set to output
	//MDIO direction set to output
	mdio_set_MDC_MDIO_direction(1,1);

	for(i=0;i<32;i++)
		mdio_1();
	mdio_start();
	mdio_rw(MDIO_RD);
	for(i=0;i<5;i++){
		if((pa>>(5-1-i))%2)
			mdio_1();
		else
			mdio_0();
	}
	for(i=0;i<5;i++){
		if((ra>>(5-1-i))%2)
			mdio_1();
		else
			mdio_0();
	}
	mdio_z();
	mdio_0();

	//MDIO DIR set to input
	mdio_set_MDC_MDIO_direction(1,0);

	for(j=0;j<16;j++){
		//MDC set to 0
		ip1829_gpio_set(GPIO_MDC_OFFSET,0);

		for(i=0;i<MDIO_DELAY;i++);
		//get MDIO value
		regBit=ip1829_gpio_get(GPIO_MDIO_OFFSET);
		if(regBit==0)
		{
			data|=0;
			debug[15-j]=0;
		}
		else
		{
			data|=1;
			debug[15-j]=1;
		}
		if(j<15)
			data=data<<1;
		//MDC set to 1
		ip1829_gpio_set(GPIO_MDC_OFFSET,1);

		for(i=0;i<MDIO_DELAY;i++);
	}
	//MDC set to 0
	ip1829_gpio_set(GPIO_MDC_OFFSET,0);

	for(i=0;i<MDIO_DELAY;i++);
	//MDC set to 1
	ip1829_gpio_set(GPIO_MDC_OFFSET,1);

	for(i=0;i<MDIO_DELAY;i++);

	return data;
}
void ic_mdio_set_divisor(unsigned long div)
{
	//do nothing
}
#else//#ifndef ACCESS_REG_BY_MDIO
void ic_mdio_set_divisor(unsigned long div)
{
	void __iomem	*base;
	unsigned long val;
	struct ip218_smictrl1 *smic1;

	FUNC_MSG_IN;

	base = (void __iomem *)(IP218_MAC_BASE + IP218_MAC_SMICTRL1);
	val = 0;
	smic1 = (struct ip218_smictrl1 *)&val;
	smic1 ->mdc_clk_divisor = div;
	writel(val, base);
	val = readl(base);
	smic1 = (struct ip218_smictrl1 *)&val;
	ip1829drv_dbg("ip218 mdio divisor=%u\n", (unsigned short)smic1 ->mdc_clk_divisor);

	FUNC_MSG_OUT;
}

void ic_mdio_wr(unsigned short pa, unsigned short ra, unsigned short va){
	void __iomem *base;
	unsigned long val;
	struct ip218_smictrl0 *smic0;

	FUNC_MSG_IN;

	val = 0;
	smic0 = (struct ip218_smictrl0 *)&val;
	smic0 -> phy = pa;
	smic0 -> reg = ra;
	smic0 -> wr_data = va;
	smic0 -> rdwr = 1;//1: write cycle
	smic0 -> en = 1;

	base = (void __iomem *)(IP218_MAC_BASE + IP218_MAC_SMICTRL0);
	ip1829drv_dbg("write [%p]=%08lX\n", base, val);
	writel(val, base);

	// wait for writing process done
	do{
		val = readl(base);
	}while(smic0 -> en);

	FUNC_MSG_OUT;
}
unsigned short ic_mdio_rd(unsigned short pa, unsigned short ra){
	void __iomem	*base;
	unsigned long val;
	unsigned short ret;
	struct ip218_smictrl0 *smic0;
	struct ip218_smictrl1 *smic1;

	FUNC_MSG_IN;

	val = 0;
	smic0 = (struct ip218_smictrl0 *)&val;
	smic0 -> phy = pa;
	smic0 -> reg = ra;
	smic0 -> rdwr = 0;//0: read cycle
	smic0 -> en = 1;

	base = (void __iomem *)(IP218_MAC_BASE + IP218_MAC_SMICTRL0);
	ip1829drv_dbg("write [%p]=%08lX\n", base, val);
	writel(val, base);

	// wait for writing process done
	do{
		val = readl(base);
	}while(smic0 -> en);

	// read data from register
	base = (void __iomem *)(IP218_MAC_BASE + IP218_MAC_SMICTRL1);
	val = readl(base);
	smic1 = (struct ip218_smictrl1 *)&val;
	ret = smic1 -> rd_data;
	ip1829drv_dbg("read [%p]=%08lX\n", base, val);

	FUNC_MSG_OUT;

	return ret;
}
#endif//ACCESS_REG_BY_MDIO

void Write_Reg(u8 regaddr, u16 value)
{
	u16 tmp = CPU_IF_SPEED_NORMAL ? regaddr : (u16)regaddr << 2;
#ifdef TEST_REG
	ip1829drv_dbg("RegList[%x][0x%02X]=0x%04x\n", pg, regaddr, RegList[pg][regaddr]);
	RegList[pg][regaddr]=value;
#else
	ic_mdio_wr(((tmp)>>5)&0x1f,(tmp)&0x1f,value);
#endif//TEST_REG

#ifdef TEST_REG
	ip1829drv_dbg("RegList[%x][0x%02X]=0x%04x\n", pg, regaddr, RegList[pg][regaddr]);
#endif//TEST_REG

	ip1829drv_dbg("write [%02X]=%04x\n", regaddr, value);
}
u16 Read_Reg(u8 regaddr)
{
	u16 u16d;
	u16 tmp = CPU_IF_SPEED_NORMAL ? regaddr : (u16)regaddr << 2;
#ifdef TEST_REG
	u16d = RegList[pg][regaddr];
#else
	u16d=ic_mdio_rd(((tmp)>>5)&0x1f,(tmp)&0x1f);
#endif//TEST_REG
#ifdef TEST_REG
	ip1829drv_dbg("RegList[%x][0x%02X]=0x%04x\n", pg, regaddr, RegList[pg][regaddr]);
#endif//TEST_REG
	ip1829drv_dbg("read [%02X]=%04x\n", regaddr, u16d);
	return u16d;
}
u16 Read_Reg_0_With_1s(void)
{
	u16 u16d;
	u16 tmp = CPU_IF_SPEED_NORMAL ? 0x300 : 0x3;
	u16d = ic_mdio_rd(((tmp)>>5)&0x1f,(tmp)&0x1f);
	ip1829drv_dbg("read [%04X]=%04x\n", tmp, u16d);
	return u16d;
}
void IP2Page(u8 page)		// Change page to pagenum
{
	u16 tmp = CPU_IF_SPEED_NORMAL ? REG_Page : (u16)REG_Page << 2;
#ifdef TEST_REG
	pg = page;
#else
	ic_mdio_wr(((tmp)>>5)&0x1f,(tmp)&0x1f,page);
#endif//TEST_REG
	ip1829drv_dbg("set to  page[%02u]\n", page);
}
/*=============   sub. driver functions   ==================*/
static void _WriteRegBits(u8 page, u16 reg, u8 offset, u8 len, u16 val)
{
	u16 mask = 0;
	u16 u16dat;

	FUNC_MSG_IN;

	IP2Page(page);
	mask = (0x1 << len) - 1;
	u16dat = Read_Reg(reg);
	u16dat &= ~(mask<<offset);
	u16dat |= (val&mask)<<offset;
	Write_Reg(reg, u16dat);

	FUNC_MSG_OUT;
}

static u16 _ReadRegBits(u8 page, u16 reg, u8 offset, u8 len)
{
	u16 mask = 0;
	u16 u16dat;
	u16 val;

	FUNC_MSG_IN;

	IP2Page(page);
	mask = (0x1 << len) - 1;
	u16dat = Read_Reg(reg);
	val = (u16dat>>offset)&mask;

	FUNC_MSG_OUT;
	return val;
}

static int _setGeneralEnable(void *cdata, int len, u8 page, u16 reg, u8 offset)
{
	int gdata;

	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	gdata = ((struct GeneralSetting *)cdata) ->gdata;
	if( gdata != OP_FUNC_ENABLE && gdata != OP_FUNC_DISABLE )
	{
		ip1829drv_dbg("Error: gdata=%X\n", gdata);
		return -EINVAL;
	}

	ip1829drv_dbg("cdata ->gdata=%d\n", gdata);

	_WriteRegBits(page, reg, offset, 1, gdata);

	FUNC_MSG_OUT;
	return 0;
}

static int _getGeneralEnable(void *cdata, int len, u8 page, u16 reg, u8 offset)
{
	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	((struct GeneralSetting *)cdata) ->gdata = _ReadRegBits(page, reg, offset, 1);

	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	FUNC_MSG_OUT;
	return 0;
}

static int _setPortmapMask(void *cdata, int len, u8 page, u8 reg)
{
	unsigned long pm;
	int pd;
	u32 u32dat;

	FUNC_MSG_IN;
	if (sizeof(struct PortmapSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	pm = ((struct PortmapSetting *)cdata) ->portmap;
	pd = ((struct PortmapSetting *)cdata) ->pmdata;
	if (pm & ~0x1FFFFFFF)
	{
		ip1829drv_dbg("Error: portmap=%08lX\n", pm);
		return -EINVAL;
	}
	if (pd & ~0x1FFFFFFF)
	{
		ip1829drv_dbg("Error: pmdata=%X\n", pd);
		return -EINVAL;
	}

	ip1829drv_dbg("portmap=0x%08lX\n", pm);
	ip1829drv_dbg("portdata=0x%08X\n", pd);

	IP2Page(page);
	u32dat = (u32)Read_Reg(reg+1)<<16 | (u32)Read_Reg(reg);

	u32dat &= ~pm;
	u32dat |= pm & pd;

	Write_Reg(reg, (u16)(u32dat & 0xFFFF));
	Write_Reg(reg+1, (u16)(u32dat >> 16));

	FUNC_MSG_OUT;
	return 0;
}

static int _getPortmapMask(void *cdata, int len, u8 page, u8 reg)
{
	unsigned long pm;
	u32 u32dat;

	FUNC_MSG_IN;
	if (sizeof(struct PortmapSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	pm = ((struct PortmapSetting *)cdata) ->portmap;
	if (pm & ~0x1FFFFFFF)
	{
		ip1829drv_dbg("Error: portmap=%08lX\n", pm);
		return -EINVAL;
	}

	IP2Page(page);
	u32dat = (u32)Read_Reg(reg+1)<<16 | (u32)Read_Reg(reg);
	((struct PortmapSetting *)cdata) ->pmdata = u32dat & pm;

	ip1829drv_dbg("cdata ->portmap=0x%08lX\n", ((struct PortmapSetting *)cdata) ->portmap);
	ip1829drv_dbg("cdata ->pmdata=0x%08X\n", ((struct PortmapSetting *)cdata) ->pmdata);
	FUNC_MSG_OUT;
	return 0;
}


static int _setPortmap(void *cdata, int len, u8 page, u8 reg)
{
	unsigned long pm;
	int pd;
	u32 u32dat;

	FUNC_MSG_IN;
	if (sizeof(struct PortmapSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	pm = ((struct PortmapSetting *)cdata) ->portmap;
	pd = ((struct PortmapSetting *)cdata) ->pmdata;

	if (pm & ~0x1FFFFFFF)
	{
		ip1829drv_dbg("Error: portmap=%08lX\n", pm);
		return -EINVAL;
	}
	if( pd != OP_FUNC_ENABLE && pd != OP_FUNC_DISABLE )
	{
		ip1829drv_dbg("Error: pd=%X\n", pd);
		return -EINVAL;
	}

	ip1829drv_dbg("portmap=0x%08lX\n", pm);
	ip1829drv_dbg("portdata=0x%X\n", pd);

	IP2Page(page);
	u32dat = (u32)Read_Reg(reg+1)<<16 | (u32)Read_Reg(reg);

	if(pd==OP_FUNC_ENABLE)
		u32dat |= pm;
	else//OP_FUNC_DISABLE
		u32dat &= ~pm;

	Write_Reg(reg, (u16)(u32dat & 0xFFFF));
	Write_Reg(reg+1, (u16)(u32dat >> 16));

	FUNC_MSG_OUT;
	return 0;
}

static int _getPortmap(void *cdata, int len, u8 page, u8 reg)
{
	unsigned long pm;
	u32 u32dat;

	FUNC_MSG_IN;
	if (sizeof(struct PortmapSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}
	pm = ((struct PortmapSetting *)cdata) ->portmap;

	if (pm & ~0x1FFFFFFF)
	{
		ip1829drv_dbg("Error: portmap=%08lX\n", pm);
		return -EINVAL;
	}
	IP2Page(page);
	u32dat = (u32)Read_Reg(reg+1)<<16 | (u32)Read_Reg(reg);
	u32dat &= pm;
	((struct PortmapSetting *)cdata) ->pmdata = u32dat;

	ip1829drv_dbg("cdata ->pmdata=0x%08X\n", ((struct PortmapSetting *)cdata) ->pmdata);
	FUNC_MSG_OUT;
	return 0;
}
/*=============   common driver functions   ==================*/
int setPortAN(void *cdata, int len)
{
	int port, an;
	u32 u32dat;

	ip1829drv_dbg("ip1829: +setPortAN...\n");
	if (sizeof(struct ByPortSetting) != len)
		return -EINVAL;

	port=((struct ByPortSetting *)cdata) ->port;
	an = ((struct ByPortSetting *)cdata) ->pdata;
	if (port < 1 || port > SWITCH_MAX_IPORT_CNT)
		return -EINVAL;
	if ( !(an==OP_FUNC_DISABLE || an==OP_FUNC_ENABLE) )
		return -EINVAL;
	ip1829drv_dbg("port=%d\n", port);
	ip1829drv_dbg("an=%d\n", an);

	IP2Page(3);
	u32dat = (u32)( (Read_Reg(P3REG_AN+1) << 16) | Read_Reg(P3REG_AN) );
	if (an == OP_FUNC_ENABLE)
		u32dat |= (u32)(1 << (port-1));
	else
		u32dat &= (u32)~(1 << (port-1));
	Write_Reg(P3REG_AN, u32dat & 0xFFFF);
	Write_Reg(P3REG_AN+1, u32dat >> 16);

	ip1829drv_dbg("ip1829: -setPortAN...\n");
	return 0;
}

int getPortAN(void *cdata, int len)
{
	int port, an;

	ip1829drv_dbg("ip1829: +getPortAN...\n");
	if (sizeof(struct ByPortSetting) != len)
		return -EINVAL;

	port = ((struct ByPortSetting *)cdata) ->port;
	if (port < 1 || port > SWITCH_MAX_IPORT_CNT)
		return -EINVAL;
	ip1829drv_dbg("port=%d\n", port);

	an = _ReadRegBits(3, P3REG_PORTSTS0 + (port-1)/2,6+((port-1)&0x1)*8 , 1);
	ip1829drv_dbg("an=0x%x\n", an);
	((struct ByPortSetting *)cdata) ->pdata = an;
	ip1829drv_dbg("cdata ->pdata=%d\n", ((struct ByPortSetting *)cdata) ->pdata);
	ip1829drv_dbg("ip1829: -getPortAN...\n");
	return 0;
}

int setPortSpeed(void *cdata, int len)
{
	int port, speed;
	u16 u16dat;
	u32 u32dat;

	ip1829drv_dbg("ip1829: +setPortSpeed...\n");
	if (sizeof(struct ByPortSetting) != len)
		return -EINVAL;

	port = ((struct ByPortSetting *)cdata) ->port;
	speed= ((struct ByPortSetting *)cdata) ->pdata;
	if (port < 1 || port > SWITCH_MAX_IPORT_CNT)
		return -EINVAL;
	if ( !(speed==OP_SMI_SPEED_10 || speed==OP_SMI_SPEED_100 ||
				(speed==OP_SMI_SPEED_1000 && port>=25)) )
		return -EINVAL;
	ip1829drv_dbg("port=%d\n", port);
	ip1829drv_dbg("speed=%d\n", speed);

	IP2Page(3);
	if (speed == OP_SMI_SPEED_1000)
	{
		u16dat = Read_Reg(P3REG_SPG);
		u16dat |= (u16)(1 << (port-25));
		Write_Reg(P3REG_SPG, u16dat);
	}
	else
	{
		if (port >= 25)
		{
			u16dat = Read_Reg(P3REG_SPG);
			u16dat &= (u16)~(1 << (port-25));
			Write_Reg(P3REG_SPG, u16dat);
		}

		u32dat = (u32)( (Read_Reg(P3REG_SP+1) << 16) | Read_Reg(P3REG_SP) );
		if (speed == OP_SMI_SPEED_100)
			u32dat |= (u32)(1 << (port-1));
		else
			u32dat &= (u32)~(1 << (port-1));
		Write_Reg(P3REG_SP, u32dat & 0xFFFF);
		Write_Reg(P3REG_SP+1, u32dat >> 16);
	}

	ip1829drv_dbg("ip1829: -setPortSpeed...\n");
	return 0;
}

int getPortSpeed(void *cdata, int len)
{
	int port, speed;

	ip1829drv_dbg("ip1829: +getPortSpeed...\n");
	if (sizeof(struct ByPortSetting) != len)
		return -EINVAL;

	port = ((struct ByPortSetting *)cdata) ->port;
	if (port < 1 || port > SWITCH_MAX_IPORT_CNT)
		return -EINVAL;
	ip1829drv_dbg("port=%d\n", port);

	speed = (int) _ReadRegBits(3, P3REG_PORTSTS0 + (port-1)/2,1+((port-1)&0x1)*8 , 2);
	ip1829drv_dbg("speed=0x%x\n", speed);
	if (speed & 0x2)	speed = OP_SMI_SPEED_1000;
	else
	{
		if (speed & 0x1)	speed = OP_SMI_SPEED_100;
		else			speed = OP_SMI_SPEED_10;
	}
	((struct ByPortSetting *)cdata) ->pdata = speed;
	ip1829drv_dbg("cdata ->pdata=%d\n", ((struct ByPortSetting *)cdata) ->pdata);
	ip1829drv_dbg("ip1829: -getPortSpeed...\n");
	return 0;
}

int setPortDuplex(void *cdata, int len)
{
	int port, dup;
	u32 u32dat;

	ip1829drv_dbg("ip1829: +setPortDuplex...\n");
	if (sizeof(struct ByPortSetting) != len)
		return -EINVAL;

	port=((struct ByPortSetting *)cdata) ->port;
	dup =((struct ByPortSetting *)cdata) ->pdata;
	if (port < 1 || port > SWITCH_MAX_IPORT_CNT)
		return -EINVAL;
	if ( !(dup==OP_SMI_DUPLEX_HALF || dup==OP_SMI_DUPLEX_FULL) )
		return -EINVAL;
	ip1829drv_dbg("port=%d\n", port);
	ip1829drv_dbg("dup=%d\n", dup);

	IP2Page(3);
	u32dat = (u32)( (Read_Reg(P3REG_DUPLEX+1) << 16) | Read_Reg(P3REG_DUPLEX) );
	if (dup == OP_SMI_DUPLEX_FULL)
		u32dat |= (u32)(1 << (port-1));
	else
		u32dat &= (u32)~(1 << (port-1));
	Write_Reg(P3REG_DUPLEX, u32dat & 0xFFFF);
	Write_Reg(P3REG_DUPLEX+1, u32dat >> 16);

	ip1829drv_dbg("ip1829: -setPortDuplex...\n");
	return 0;
}

int getPortDuplex(void *cdata, int len)
{
	int port, dup;

	ip1829drv_dbg("ip1829: +getPortDuplex...\n");
	if (sizeof(struct ByPortSetting) != len)
		return -EINVAL;

	port = ((struct ByPortSetting *)cdata) ->port;
	if (port < 1 || port > SWITCH_MAX_IPORT_CNT)
		return -EINVAL;
	ip1829drv_dbg("port=%d\n", port);

	dup = (int) _ReadRegBits(3, P3REG_PORTSTS0 + (port-1)/2,(3+((port-1)&0x1)*8), 1);
	ip1829drv_dbg("dup=0x%x\n", dup);
	((struct ByPortSetting *)cdata) ->pdata = dup;
	ip1829drv_dbg("cdata ->pdata=%d\n", ((struct ByPortSetting *)cdata) ->pdata);
	ip1829drv_dbg("ip1829: -getPortDuplex...\n");
	return 0;
}

int setPortPause(void *cdata, int len)
{
	int port, ps;
	u32 u32dat;

	ip1829drv_dbg("ip1829: +setPortPause...\n");
	if (sizeof(struct ByPortSetting) != len)
		return -EINVAL;

	port=((struct ByPortSetting *)cdata) ->port;
	ps = ((struct ByPortSetting *)cdata) ->pdata;
	if (port < 1 || port > SWITCH_MAX_IPORT_CNT)
		return -EINVAL;
	if ( !(ps==OP_FUNC_ENABLE || ps==OP_FUNC_DISABLE) )
		return -EINVAL;
	ip1829drv_dbg("port=%d\n", port);
	ip1829drv_dbg("ps=%d\n", ps);

	IP2Page(3);
	u32dat = (u32)( (Read_Reg(P3REG_PAUSE+1) << 16) | Read_Reg(P3REG_PAUSE) );
	if (ps == OP_FUNC_ENABLE)
		u32dat |= (u32)(1 << (port-1));
	else
		u32dat &= (u32)~(1 << (port-1));
	Write_Reg(P3REG_PAUSE, u32dat & 0xFFFF);
	Write_Reg(P3REG_PAUSE+1, u32dat >> 16);

	ip1829drv_dbg("ip1829: -setPortPause...\n");
	return 0;
}

int getPortPause(void *cdata, int len)
{
	int port, ps;

	ip1829drv_dbg("ip1829: +getPortPause...\n");
	if (sizeof(struct ByPortSetting) != len)
		return -EINVAL;

	port = ((struct ByPortSetting *)cdata) ->port;
	if (port < 1 || port > SWITCH_MAX_IPORT_CNT)
		return -EINVAL;
	ip1829drv_dbg("port=%d\n", port);

	ps = (int) _ReadRegBits(3, P3REG_PORTSTS0 + (port-1)/2,4+((port-1)&0x1)*8 , 1);
	ip1829drv_dbg("ps=0x%x\n", ps);
	((struct ByPortSetting *)cdata) ->pdata = ps;
	ip1829drv_dbg("cdata ->pdata=%d\n", ((struct ByPortSetting *)cdata) ->pdata);
	ip1829drv_dbg("ip1829: -getPortPause...\n");
	return 0;
}

int setPortAsymPause(void *cdata, int len)
{
	int port, aps;
	u32 u32dat;

	ip1829drv_dbg("ip1829: +setPortAsymPause...\n");
	if (sizeof(struct ByPortSetting) != len)
		return -EINVAL;

	port=((struct ByPortSetting *)cdata) ->port;
	aps =((struct ByPortSetting *)cdata) ->pdata;
	if (port < 1 || port > SWITCH_MAX_IPORT_CNT)
		return -EINVAL;
	if ( !(aps==OP_FUNC_ENABLE || aps==OP_FUNC_DISABLE) )
		return -EINVAL;
	ip1829drv_dbg("port=%d\n", port);
	ip1829drv_dbg("aps=%d\n", aps);

	IP2Page(3);
	u32dat = (u32)( (Read_Reg(P3REG_ASPAUSE+1) << 16) | Read_Reg(P3REG_ASPAUSE) );
	if (aps == OP_FUNC_ENABLE)
		u32dat |= (u32)(1 << (port-1));
	else
		u32dat &= (u32)~(1 << (port-1));
	Write_Reg(P3REG_ASPAUSE, u32dat & 0xFFFF);
	Write_Reg(P3REG_ASPAUSE+1, u32dat >> 16);

	ip1829drv_dbg("ip1829: -setPortAsymPause...\n");
	return 0;
}

int getPortAsymPause(void *cdata, int len)
{
	int port, aps;

	ip1829drv_dbg("ip1829: +getPortAsymPause...\n");
	if (sizeof(struct ByPortSetting) != len)
		return -EINVAL;

	port = ((struct ByPortSetting *)cdata) ->port;
	if (port < 1 || port > SWITCH_MAX_IPORT_CNT)
		return -EINVAL;
	ip1829drv_dbg("port=%d\n", port);

	aps = (int) _ReadRegBits(3, P3REG_PORTSTS0 + (port-1)/2,5+((port-1)&0x1)*8 , 1);
	ip1829drv_dbg("aps=0x%x\n", aps);
	((struct ByPortSetting *)cdata) ->pdata = aps;
	ip1829drv_dbg("cdata ->pdata=%d\n", ((struct ByPortSetting *)cdata) ->pdata);
	ip1829drv_dbg("ip1829: -getPortAsymPause...\n");
	return 0;
}

int setPortLinkStatus(void *cdata, int len)
{
	int port, sp, dup, ps, asym, an;
	u16 u16dat;
	u32 u32dat;

	ip1829drv_dbg("ip1829: +setPortLinkStatus...\n");
	if (sizeof(struct LinkStatusSetting) != len)
		return -EINVAL;

	port=((struct LinkStatusSetting *)cdata) ->port;
	sp = ((struct LinkStatusSetting *)cdata) ->speed;
	dup =((struct LinkStatusSetting *)cdata) ->duplex;
	ps = ((struct LinkStatusSetting *)cdata) ->pause;
	asym=((struct LinkStatusSetting *)cdata) ->asym;
	an = ((struct LinkStatusSetting *)cdata) ->an;
	if (port < 1 || port > SWITCH_MAX_IPORT_CNT)
		return -EINVAL;
	if ( !(sp==OP_SMI_SPEED_10 || sp==OP_SMI_SPEED_100 ||
				(sp==OP_SMI_SPEED_1000 && port>=25)) )
		return -EINVAL;
	if ( !(dup==OP_SMI_DUPLEX_HALF || dup==OP_SMI_DUPLEX_FULL) )
		return -EINVAL;
	if ( !(ps==OP_FUNC_DISABLE || ps==OP_FUNC_ENABLE) )
		return -EINVAL;
	if ( !(asym==OP_FUNC_DISABLE || asym==OP_FUNC_ENABLE) )
		return -EINVAL;
	if ( !(an==OP_FUNC_DISABLE || an==OP_FUNC_ENABLE) )
		return -EINVAL;
	ip1829drv_dbg("port=%d\n", port);
	ip1829drv_dbg("sp=%d  dup=%d  ps=%d  asym=%d  an=%d\n",
			sp, dup, ps, asym, an);

	IP2Page(3);
	if (sp == OP_SMI_SPEED_1000)
	{
		u16dat = Read_Reg(P3REG_SPG);
		u16dat |= (u16)(1 << (port-25));
		Write_Reg(P3REG_SPG, u16dat);
	}
	else
	{
		if (port >= 25)
		{
			u16dat = Read_Reg(P3REG_SPG);
			u16dat &= (u16)~(1 << (port-25));
			Write_Reg(P3REG_SPG, u16dat);
		}

		u32dat = (u32)( (Read_Reg(P3REG_SP+1) << 16) | Read_Reg(P3REG_SP) );
		if (sp == OP_SMI_SPEED_100)
			u32dat |= (u32)(1 << (port-1));
		else
			u32dat &= (u32)~(1 << (port-1));
		Write_Reg(P3REG_SP, u32dat & 0xFFFF);
		Write_Reg(P3REG_SP+1, u32dat >> 16);
	}

	u32dat = (u32)( (Read_Reg(P3REG_DUPLEX+1) << 16) | Read_Reg(P3REG_DUPLEX) );
	if (dup == OP_SMI_DUPLEX_FULL)
		u32dat |= (u32)(1 << (port-1));
	else
		u32dat &= (u32)~(1 << (port-1));
	Write_Reg(P3REG_DUPLEX, u32dat & 0xFFFF);
	Write_Reg(P3REG_DUPLEX+1, u32dat >> 16);

	u32dat = (u32)( (Read_Reg(P3REG_PAUSE+1) << 16) | Read_Reg(P3REG_PAUSE) );
	if (ps == OP_FUNC_ENABLE)
		u32dat |= (u32)(1 << (port-1));
	else
		u32dat &= (u32)~(1 << (port-1));
	Write_Reg(P3REG_PAUSE, u32dat & 0xFFFF);
	Write_Reg(P3REG_PAUSE+1, u32dat >> 16);

	u32dat = (u32)( (Read_Reg(P3REG_ASPAUSE+1) << 16) | Read_Reg(P3REG_ASPAUSE) );
	if (asym == OP_FUNC_ENABLE)
		u32dat |= (u32)(1 << (port-1));
	else
		u32dat &= (u32)~(1 << (port-1));
	Write_Reg(P3REG_ASPAUSE, u32dat & 0xFFFF);
	Write_Reg(P3REG_ASPAUSE+1, u32dat >> 16);

	u32dat = (u32)( (Read_Reg(P3REG_AN+1) << 16) | Read_Reg(P3REG_AN) );
	if (an == OP_FUNC_ENABLE)
		u32dat |= (u32)(1 << (port-1));
	else
		u32dat &= (u32)~(1 << (port-1));
	Write_Reg(P3REG_AN, u32dat & 0xFFFF);
	Write_Reg(P3REG_AN+1, u32dat >> 16);

	ip1829drv_dbg("ip1829: -setPortLinkStatus...\n");
	return 0;
}

int getPortLinkStatus(void *cdata, int len)
{
	int port, link, sp, dup, ps, asym, an;
	u8  u8dat;
	u16 u16dat;

	ip1829drv_dbg("ip1829: +getPortLinkStatus...\n");
	if (sizeof(struct LinkStatusSetting) != len)
		return -EINVAL;

	port = ((struct LinkStatusSetting *)cdata) ->port;
	if (port < 1 || port > SWITCH_MAX_IPORT_CNT)
		return -EINVAL;
	ip1829drv_dbg("port=%d\n", port);

	IP2Page(3);
	u8dat = P3REG_PORTSTS0 + (port-1)/2;
	u16dat = Read_Reg(u8dat);
	ip1829drv_dbg("u16dat=0x%04x\n", u16dat);
	link=(int)( ( u16dat >> (0+((port-1)&0x1)*8) ) & 0x1 );
	sp = (int)( ( u16dat >> (1+((port-1)&0x1)*8) ) & 0x3 );
	if (sp & 0x2)	sp = OP_SMI_SPEED_1000;
	else
	{
		if (sp & 0x1)	sp = OP_SMI_SPEED_100;
		else			sp = OP_SMI_SPEED_10;
	}
	dup =(int)( ( u16dat >> (3+((port-1)&0x1)*8) ) & 0x1 );
	ps = (int)( ( u16dat >> (4+((port-1)&0x1)*8) ) & 0x1 );
	asym=(int)( ( u16dat >> (5+((port-1)&0x1)*8) ) & 0x1 );
	an = (int)( ( u16dat >> (6+((port-1)&0x1)*8) ) & 0x1 );
	ip1829drv_dbg("link=0x%x  sp=%d  dup=0x%x  ps=0x%x  asym=0x%x  an=0x%x\n",
			link, sp, dup, ps, asym, an);
	((struct LinkStatusSetting *)cdata) ->link = link;
	((struct LinkStatusSetting *)cdata) ->speed = sp;
	((struct LinkStatusSetting *)cdata) ->duplex = dup;
	((struct LinkStatusSetting *)cdata) ->pause = ps;
	((struct LinkStatusSetting *)cdata) ->asym = asym;
	((struct LinkStatusSetting *)cdata) ->an = an;
	ip1829drv_dbg("cdata ->link=%d\n", ((struct LinkStatusSetting *)cdata) ->link);
	ip1829drv_dbg("cdata ->speed=%d\n", ((struct LinkStatusSetting *)cdata) ->speed);
	ip1829drv_dbg("cdata ->duplex=%d\n", ((struct LinkStatusSetting *)cdata) ->duplex);
	ip1829drv_dbg("cdata ->pause=%d\n", ((struct LinkStatusSetting *)cdata) ->pause);
	ip1829drv_dbg("cdata ->asym=%d\n", ((struct LinkStatusSetting *)cdata) ->asym);
	ip1829drv_dbg("cdata ->an=%d\n", ((struct LinkStatusSetting *)cdata) ->an);
	ip1829drv_dbg("ip1829: -getPortLinkStatus...\n");
	return 0;
}

int setPortBackpressure(void *cdata, int len)
{
	int port, bp;
	u32 u32dat;

	ip1829drv_dbg("ip1829: +setPortBackpressure...\n");
	if (sizeof(struct ByPortSetting) != len)
		return -EINVAL;

	port=((struct ByPortSetting *)cdata) ->port;
	bp = ((struct ByPortSetting *)cdata) ->pdata;
	if (port < 1 || port > SWITCH_MAX_IPORT_CNT)
		return -EINVAL;
	if ( !(bp==OP_FUNC_ENABLE || bp==OP_FUNC_DISABLE) )
		return -EINVAL;
	ip1829drv_dbg("port=%d\n", port);
	ip1829drv_dbg("bp=%d\n", bp);

	IP2Page(3);
	u32dat = (u32)( (Read_Reg(P3REG_BPRESS+1) << 16) | Read_Reg(P3REG_BPRESS) );
	if (bp == OP_FUNC_ENABLE)
		u32dat |= (u32)(1 << (port-1));
	else
		u32dat &= (u32)~(1 << (port-1));
	Write_Reg(P3REG_BPRESS, u32dat & 0xFFFF);
	Write_Reg(P3REG_BPRESS+1, u32dat >> 16);

	ip1829drv_dbg("ip1829: -setPortBackpressure...\n");
	return 0;
}

int getPortBackpressure(void *cdata, int len)
{
	int port, bp;

	ip1829drv_dbg("ip1829: +getPortBackpressure...\n");
	if (sizeof(struct ByPortSetting) != len)
		return -EINVAL;

	port = ((struct ByPortSetting *)cdata) ->port;
	if (port < 1 || port > SWITCH_MAX_IPORT_CNT)
		return -EINVAL;
	ip1829drv_dbg("port=%d\n", port);

	bp = (int)_ReadRegBits(3, P3REG_BPRESS + (port-1)/16, (port-1)%16, 1);
	ip1829drv_dbg("bp=0x%x\n", bp);
	((struct ByPortSetting *)cdata) ->pdata = bp;
	ip1829drv_dbg("cdata ->pdata=%d\n", ((struct ByPortSetting *)cdata) ->pdata);
	ip1829drv_dbg("ip1829: -getPortBackpressure...\n");
	return 0;
}

int setPortPowerDown(void *cdata, int len)
{
	int port, pd;
	u32 u32dat;

	ip1829drv_dbg("ip1829: +setPortPowerDown...\n");
	if (sizeof(struct ByPortSetting) != len)
		return -EINVAL;

	port=((struct ByPortSetting *)cdata) ->port;
	pd = ((struct ByPortSetting *)cdata) ->pdata;
	if (port < 1 || port > SWITCH_MAX_IPORT_CNT)
		return -EINVAL;
	if ( !(pd==OP_FUNC_ENABLE || pd==OP_FUNC_DISABLE) )
		return -EINVAL;
	ip1829drv_dbg("port=%d\n", port);
	ip1829drv_dbg("pd=%d\n", pd);

	IP2Page(3);
	u32dat = (u32)( (Read_Reg(P3REG_POWERDOWN+1) << 16) | Read_Reg(P3REG_POWERDOWN) );
	if (pd == OP_FUNC_ENABLE)
		u32dat |= (u32)(1 << (port-1));
	else
		u32dat &= (u32)~(1 << (port-1));
	Write_Reg(P3REG_POWERDOWN, u32dat & 0xFFFF);
	Write_Reg(P3REG_POWERDOWN+1, u32dat >> 16);

	ip1829drv_dbg("ip1829: -setPortPowerDown...\n");
	return 0;
}

int getPortPowerDown(void *cdata, int len)
{
	int port, pd;

	ip1829drv_dbg("ip1829: +getPortPowerDown...\n");
	if (sizeof(struct ByPortSetting) != len)
		return -EINVAL;

	port = ((struct ByPortSetting *)cdata) ->port;
	if (port < 1 || port > SWITCH_MAX_IPORT_CNT)
		return -EINVAL;
	ip1829drv_dbg("port=%d\n", port);

	pd = (int)_ReadRegBits(3, P3REG_POWERDOWN + (port-1)/16, (port-1)%16, 1);
	ip1829drv_dbg("pd=0x%x\n", pd);
	((struct ByPortSetting *)cdata) ->pdata = pd;
	ip1829drv_dbg("cdata ->pdata=%d\n", ((struct ByPortSetting *)cdata) ->pdata);
	ip1829drv_dbg("ip1829: -getPortPowerDown...\n");
	return 0;
}

int setPortForceLink(void *cdata, int len)
{
	int port, fl;
	u32 u32dat;

	ip1829drv_dbg("ip1829: +setPortForceLink...\n");
	if (sizeof(struct ByPortSetting) != len)
		return -EINVAL;

	port=((struct ByPortSetting *)cdata) ->port;
	fl = ((struct ByPortSetting *)cdata) ->pdata;
	if (port < 1 || port > (SWITCH_MAX_IPORT_CNT-1))
		return -EINVAL;
	if ( !(fl==OP_FUNC_ENABLE || fl==OP_FUNC_DISABLE) )
		return -EINVAL;
	ip1829drv_dbg("port=%d\n", port);
	ip1829drv_dbg("fl=%d\n", fl);

	IP2Page(3);
	u32dat = (u32)( (Read_Reg(P3REG_FORCELINK+1) << 16) | Read_Reg(P3REG_FORCELINK) );
	if (fl == OP_FUNC_ENABLE)
		u32dat |= (u32)(1 << (port-1));
	else
		u32dat &= (u32)~(1 << (port-1));
	Write_Reg(P3REG_FORCELINK, u32dat & 0xFFFF);
	Write_Reg(P3REG_FORCELINK+1, u32dat >> 16);

	ip1829drv_dbg("ip1829: -setPortForceLink...\n");
	return 0;
}

int getPortForceLink(void *cdata, int len)
{
	int port, fl;

	ip1829drv_dbg("ip1829: +getPortForceLink...\n");
	if (sizeof(struct ByPortSetting) != len)
		return -EINVAL;

	port = ((struct ByPortSetting *)cdata) ->port;
	if (port < 1 || port > (SWITCH_MAX_IPORT_CNT-1))
		return -EINVAL;
	ip1829drv_dbg("port=%d\n", port);

	fl = (int)_ReadRegBits(3, P3REG_FORCELINK + (port-1)/16, (port-1)%16, 1);

	ip1829drv_dbg("fl=0x%x\n", fl);
	((struct ByPortSetting *)cdata) ->pdata = fl;
	ip1829drv_dbg("cdata ->pdata=%d\n", ((struct ByPortSetting *)cdata) ->pdata);
	ip1829drv_dbg("ip1829: -getPortForceLink...\n");
	return 0;
}

int setPortUniDirection(void *cdata, int len)
{
	int port, uni;
	u32 u32dat;

	ip1829drv_dbg("ip1829: +setPortUniDirection...\n");
	if (sizeof(struct ByPortSetting) != len)
		return -EINVAL;

	port=((struct ByPortSetting *)cdata) ->port;
	uni =((struct ByPortSetting *)cdata) ->pdata;
	if (port < 1 || port > SWITCH_MAX_IPORT_CNT)
		return -EINVAL;
	if ( !(uni==OP_FUNC_ENABLE || uni==OP_FUNC_DISABLE) )
		return -EINVAL;
	ip1829drv_dbg("port=%d\n", port);
	ip1829drv_dbg("uni=%d\n", uni);

	IP2Page(3);
	u32dat = (u32)( (Read_Reg(P3REG_UNIDIRECT+1) << 16) | Read_Reg(P3REG_UNIDIRECT) );
	if (uni == OP_FUNC_ENABLE)
		u32dat |= (u32)(1 << (port-1));
	else
		u32dat &= (u32)~(1 << (port-1));
	Write_Reg(P3REG_UNIDIRECT, u32dat & 0xFFFF);
	Write_Reg(P3REG_UNIDIRECT+1, u32dat >> 16);

	ip1829drv_dbg("ip1829: -setPortUniDirection...\n");
	return 0;
}

int getPortUniDirection(void *cdata, int len)
{
	int port, uni;

	ip1829drv_dbg("ip1829: +getPortUniDirection...\n");
	if (sizeof(struct ByPortSetting) != len)
		return -EINVAL;

	port = ((struct ByPortSetting *)cdata) ->port;
	if (port < 1 || port > SWITCH_MAX_IPORT_CNT)
		return -EINVAL;
	ip1829drv_dbg("port=%d\n", port);

	uni = (int)_ReadRegBits(3, P3REG_UNIDIRECT + (port-1)/16, (port-1)%16, 1);

	ip1829drv_dbg("uni=0x%x\n", uni);
	((struct ByPortSetting *)cdata) ->pdata = uni;
	ip1829drv_dbg("cdata ->pdata=%d\n", ((struct ByPortSetting *)cdata) ->pdata);
	ip1829drv_dbg("ip1829: -getPortUniDirection...\n");
	return 0;
}

//------------ Capture functions:common ----------------------------
int setL2CapAct(void *cdata, int len)
{
	int ptcl, act;
	u32 u32dat, mask;

	ip1829drv_dbg("ip1829: +setL2CapAct...\n");
	if (sizeof(struct CapActSetting) != len)
		return -EINVAL;

	ptcl= ((struct CapActSetting *)cdata) ->protocol;
	act = ((struct CapActSetting *)cdata) ->act;
	if (ptcl < OP_CAP_PTCL_BPDU || ptcl > OP_CAP_PTCL_GRP3)
		return -EINVAL;
	if (act!=OP_CAP_ACT_FORWARD && act!=OP_CAP_ACT_TO_CPU && act!=OP_CAP_ACT_DROP)
		return -EINVAL;
	ip1829drv_dbg("ptcl=%d\n", ptcl);
	ip1829drv_dbg("act=%d\n", act);

	IP2Page(0);
	u32dat = ( Read_Reg(P0REG_L2FRAMEGETCTRL1) << 16 | Read_Reg(P0REG_L2FRAMEGETCTRL) );
	mask = (u32)~(0x3 << (ptcl*2));
	u32dat &= mask;
	u32dat |= (u32)(act << (ptcl*2));
	Write_Reg(P0REG_L2FRAMEGETCTRL, (u16)(u32dat & 0xFFFF));
	Write_Reg(P0REG_L2FRAMEGETCTRL1, (u16)(u32dat >> 16));

	ip1829drv_dbg("ip1829: -setL2CapAct...\n");
	return 0;
}

int getL2CapAct(void *cdata, int len)
{
	int ptcl, act;
	u32 u32dat;

	ip1829drv_dbg("ip1829: +getL2CapAct...\n");
	if (sizeof(struct CapActSetting) != len)
		return -EINVAL;

	ptcl= ((struct CapActSetting *)cdata) ->protocol;
	if (ptcl < OP_CAP_PTCL_BPDU || ptcl > OP_CAP_PTCL_GRP3)
		return -EINVAL;
	ip1829drv_dbg("ptcl=%d\n", ptcl);

	IP2Page(0);
	u32dat = ( Read_Reg(P0REG_L2FRAMEGETCTRL1) << 16 | Read_Reg(P0REG_L2FRAMEGETCTRL) );
	act = (int)((u32dat >> (ptcl*2)) & 0x3);
	((struct CapActSetting *)cdata) ->act = act;
	ip1829drv_dbg("cdata ->act=%d\n", ((struct CapActSetting *)cdata) ->act);
	ip1829drv_dbg("ip1829: -getL2CapAct...\n");
	return 0;
}

int setCapInBand(void *cdata, int len)
{
	int gdata;

	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	gdata = ((struct GeneralSetting *)cdata) ->gdata;
	if( gdata != OP_CAP_ACT_TO_CPU && gdata != OP_CAP_ACT_DROP )
	{
		ip1829drv_dbg("Error: gdata=%X\n", gdata);
		return -EINVAL;
	}

	ip1829drv_dbg("cdata ->gdata=%d\n", gdata);

	_WriteRegBits(0, P0REG_MACBEHAVIOR, 6, 1, (gdata==OP_CAP_ACT_TO_CPU?0x1:0x0));

	FUNC_MSG_OUT;
	return 0;
}

int getCapInBand(void *cdata, int len)
{
	int gdata;

	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	gdata = _ReadRegBits(0, P0REG_MACBEHAVIOR, 6, 1)?OP_CAP_ACT_TO_CPU:OP_CAP_ACT_DROP;
	((struct GeneralSetting *)cdata) ->gdata = gdata;

	ip1829drv_dbg("cdata ->gdata=0x%08x\n", ((struct GeneralSetting *)cdata) ->gdata);
	FUNC_MSG_OUT;
	return 0;
}

int setCapInBandRestrict(void *cdata, int len)
{
	int gdata;

	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	gdata = ((struct GeneralSetting *)cdata) ->gdata;
	if( gdata != OP_FUNC_ENABLE && gdata != OP_FUNC_DISABLE )
	{
		ip1829drv_dbg("Error: gdata=%X\n", gdata);
		return -EINVAL;
	}

	ip1829drv_dbg("cdata ->gdata=%d\n", gdata);

	_WriteRegBits(0, P0REG_INBANDRESTRICT1, 15, 1, (gdata==OP_FUNC_ENABLE?0x1:0x0));

	FUNC_MSG_OUT;
	return 0;
}

int getCapInBandRestrict(void *cdata, int len)
{
	int gdata;

	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	gdata = _ReadRegBits(0, P0REG_INBANDRESTRICT1, 15, 1)?OP_FUNC_ENABLE:OP_FUNC_DISABLE;
	((struct GeneralSetting *)cdata) ->gdata = gdata;

	ip1829drv_dbg("cdata ->gdata=0x%08x\n", ((struct GeneralSetting *)cdata) ->gdata);
	FUNC_MSG_OUT;
	return 0;
}

int setCapInBandRestrictCfg(void *cdata, int len)
{
	unsigned int ptcl;
	int act;

	FUNC_MSG_IN;
	if (sizeof(struct CapActSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	ptcl = ((struct CapActSetting *)cdata) ->protocol;
	act = ((struct CapActSetting *)cdata) ->act;

	if( ptcl >= OP_CAT_INBAND_TOTALNUM)
	{
		ip1829drv_dbg("Error: protocol=%X\n", ptcl);
		return -EINVAL;
	}

	if( (act != OP_CAP_ACT_TO_CPU) && (act != OP_CAP_ACT_DROP))
	{
		ip1829drv_dbg("Error: act=%X\n", act);
		return -EINVAL;
	}

	ip1829drv_dbg("cdata ->ptcl=%X\n", ptcl);
	ip1829drv_dbg("cdata ->act=%X\n", act);

	_WriteRegBits(0, P0REG_INBANDRESTRICT+(ptcl/16), (ptcl % 16), 1, (act==OP_CAP_ACT_TO_CPU?0x1:0x0));

	FUNC_MSG_OUT;
	return 0;
}

int getCapInBandRestrictCfg(void *cdata, int len)
{
	unsigned int ptcl;
	int act;

	FUNC_MSG_IN;
	if (sizeof(struct CapActSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	ptcl = ((struct CapActSetting *)cdata) ->protocol;

	if( ptcl >= OP_CAT_INBAND_TOTALNUM)
	{
		ip1829drv_dbg("Error: protocol=%X\n", ptcl);
		return -EINVAL;
	}

	act = _ReadRegBits(0, P0REG_INBANDRESTRICT+(ptcl/16), (ptcl % 16), 1)?OP_CAP_ACT_TO_CPU:OP_CAP_ACT_DROP;
	((struct CapActSetting *)cdata) ->act = act;

	ip1829drv_dbg("cdata ->protocol=%X\n", ((struct CapActSetting *)cdata) ->protocol);
	ip1829drv_dbg("cdata ->act=%X\n", ((struct CapActSetting *)cdata) ->act);
	FUNC_MSG_OUT;
	return 0;
}

int setCapSwitchMac(void *cdata, int len)
{
	char mac[6];
	int i;
	u16 u16dat;

	FUNC_MSG_IN;
	if (sizeof(struct MACSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	memcpy(mac, ((struct MACSetting *)cdata) ->mac, 6);
	for(i=0; i<6; i++)
		ip1829drv_dbg("cdata ->mac[%d]=%X\n", i, mac[i]);
	IP2Page(0);
	for(i=0; i<3; i++)
	{
		u16dat = ((((u16)mac[i*2])&0xFF)<<8) | (((u16)mac[i*2+1])&0xFF);
		Write_Reg(P0REG_MACADDRESS+(2-i), u16dat);
	}
	FUNC_MSG_OUT;
	return 0;
}

int getCapSwitchMac(void *cdata, int len)
{
	char mac[6];
	int i;
	u16 u16dat;

	FUNC_MSG_IN;
	if (sizeof(struct MACSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	IP2Page(0);
	for(i=0; i<3; i++)
	{
		u16dat = Read_Reg(P0REG_MACADDRESS+(2-i));
		mac[i*2] = (u16dat>>8)&0xFF;
		mac[i*2+1] = u16dat&0xFF;
	}
	memcpy(((struct MACSetting *)cdata) ->mac, mac, 6);

	for(i=0; i<6; i++)
		ip1829drv_dbg("cdata ->mac[%d]=%X\n", i, ((struct MACSetting *)cdata) ->mac[i]);
	FUNC_MSG_OUT;
	return 0;
}

int setCapL3Act(void *cdata, int len)
{
	unsigned int ptcl;
	int act;

	FUNC_MSG_IN;
	if (sizeof(struct CapActSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	ptcl = ((struct CapActSetting *)cdata) ->protocol;
	act = ((struct CapActSetting *)cdata) ->act;

	if( ptcl >= OP_CAT_L3_TOTALNUM)
	{
		ip1829drv_dbg("Error: protocol=%X\n", ptcl);
		return -EINVAL;
	}

	if( (act != OP_CAP_ACT_FORWARD) && (act != OP_CAP_ACT_ALL_PORT) && (act != OP_CAP_ACT_TO_CPU) && (act != OP_CAP_ACT_DROP))
	{
		ip1829drv_dbg("Error: act=%X\n", act);
		return -EINVAL;
	}

	ip1829drv_dbg("cdata ->ptcl=%X\n", ptcl);
	ip1829drv_dbg("cdata ->act=%X\n", act);

	_WriteRegBits(0, P0REG_L3FRAMEGETCTRL, (ptcl*2), 2, act);

	FUNC_MSG_OUT;
	return 0;
}

int getCapL3Act(void *cdata, int len)
{
	unsigned int ptcl;
	int act;

	FUNC_MSG_IN;
	if (sizeof(struct CapActSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	ptcl = ((struct CapActSetting *)cdata) ->protocol;

	if( ptcl >= OP_CAT_L3_TOTALNUM)
	{
		ip1829drv_dbg("Error: protocol=%X\n", ptcl);
		return -EINVAL;
	}

	act = _ReadRegBits(0, P0REG_L3FRAMEGETCTRL, (ptcl*2), 2);
	((struct CapActSetting *)cdata) ->act = act;

	ip1829drv_dbg("cdata ->protocol=%X\n", ((struct CapActSetting *)cdata) ->protocol);
	ip1829drv_dbg("cdata ->act=%X\n", ((struct CapActSetting *)cdata) ->act);
	FUNC_MSG_OUT;
	return 0;
}

int setCapL3User(void *cdata, int len)
{
	unsigned int ptcl;
	int act;

	FUNC_MSG_IN;
	if (sizeof(struct CapActSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	ptcl = ((struct CapActSetting *)cdata) ->protocol;
	act = ((struct CapActSetting *)cdata) ->act;

	if( ptcl != OP_CAT_L3_USR1 && ptcl != OP_CAT_L3_USR2)
	{
		ip1829drv_dbg("Error: protocol=%X\n", ptcl);
		return -EINVAL;
	}

	if( act < 0x0 || act > 0xFF)
	{
		ip1829drv_dbg("Error: act=%X\n", act);
		return -EINVAL;
	}

	ip1829drv_dbg("cdata ->ptcl=%X\n", ptcl);
	ip1829drv_dbg("cdata ->act=%X\n", act);

	_WriteRegBits(0, P0REG_L3USERDEF, (ptcl==OP_CAT_L3_USR1?0:8), 8, act);

	FUNC_MSG_OUT;
	return 0;
}

int getCapL3User(void *cdata, int len)
{
	unsigned int ptcl;
	int act;

	FUNC_MSG_IN;
	if (sizeof(struct CapActSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	ptcl = ((struct CapActSetting *)cdata) ->protocol;

	if( ptcl != OP_CAT_L3_USR1 && ptcl != OP_CAT_L3_USR2)
	{
		ip1829drv_dbg("Error: protocol=%X\n", ptcl);
		return -EINVAL;
	}

	act = _ReadRegBits(0, P0REG_L3USERDEF, (ptcl==OP_CAT_L3_USR1?0:8), 8);
	((struct CapActSetting *)cdata) ->act = act;

	ip1829drv_dbg("cdata ->protocol=%X\n", ((struct CapActSetting *)cdata) ->protocol);
	ip1829drv_dbg("cdata ->act=%X\n", ((struct CapActSetting *)cdata) ->act);
	FUNC_MSG_OUT;
	return 0;
}

int setCapEtherUser(void *cdata, int len)
{
	int type;

	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	type = ((struct GeneralSetting *)cdata) ->gdata;
	if( type < 0 || type >0xFFFF )
	{
		ip1829drv_dbg("Error: type=%X\n", type);
		return -EINVAL;
	}

	ip1829drv_dbg("cdata ->gdata=%d\n", type);

	IP2Page(0);

	Write_Reg(P0REG_ETHERUSERDEF, (u16)type);

	FUNC_MSG_OUT;
	return 0;
}

int getCapEtherUser(void *cdata, int len)
{
	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	IP2Page(0);
	((struct GeneralSetting *)cdata) ->gdata = (int)Read_Reg(P0REG_ETHERUSERDEF);

	ip1829drv_dbg("cdata ->gdata=0x%08X\n", ((struct GeneralSetting *)cdata) ->gdata);
	FUNC_MSG_OUT;
	return 0;
}

int setCapIpv6TcpUdpEnable(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _setGeneralEnable(cdata, len, 0, P0REG_IPV6RLTCFG, 0);
	FUNC_MSG_OUT;
	return ret;
}

int getCapIpv6TcpUdpEnable(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _getGeneralEnable(cdata, len, 0, P0REG_IPV6RLTCFG, 0);
	FUNC_MSG_OUT;
	return ret;
}

int setCapIpv6TcpUdpFlagEnable(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _setGeneralEnable(cdata, len, 0, P0REG_IPV6RLTCFG, 1);
	FUNC_MSG_OUT;
	return ret;
}

int getCapIpv6TcpUdpFlagEnable(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _getGeneralEnable(cdata, len, 0, P0REG_IPV6RLTCFG, 1);
	FUNC_MSG_OUT;
	return ret;
}

int setCapIpv6StopFinding(void *cdata, int len)
{
	unsigned int header;
	int act;

	FUNC_MSG_IN;
	if (sizeof(struct IPv6Setting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	header = ((struct IPv6Setting *)cdata) ->header;
	if( header >  OP_IPV6_HEADER_USER2
			||	header == OP_IPV6_HEADER_ICMPV6
			||	header == OP_IPV6_HEADER_ICMPV6_MLD
			||	header == OP_IPV6_HEADER_ICMPV6_NDP )
	{
		ip1829drv_dbg("Error: header=%X\n", header);
		return -EINVAL;
	}

	act = ((struct IPv6Setting *)cdata) ->act;
	if( act != OP_FUNC_ENABLE && act != OP_FUNC_DISABLE )
	{
		ip1829drv_dbg("Error: act=%X\n", act);
		return -EINVAL;
	}

	ip1829drv_dbg("cdata ->header=%X\n", header);
	ip1829drv_dbg("cdata ->act=%X\n", act);

	_WriteRegBits(0, P0REG_IPV6RLTCFG, (header<=OP_IPV6_HEADER_AUTH)?(5-header):(header+1), 1, act);

	FUNC_MSG_OUT;
	return 0;
}

int getCapIpv6StopFinding(void *cdata, int len)
{
	unsigned int header;
	int act;

	FUNC_MSG_IN;
	if (sizeof(struct IPv6Setting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	header = ((struct IPv6Setting *)cdata) ->header;
	if( header >  OP_IPV6_HEADER_USER2
			||	header == OP_IPV6_HEADER_ICMPV6_MLD
			||	header == OP_IPV6_HEADER_ICMPV6_NDP )
	{
		ip1829drv_dbg("Error: header=%X\n", header);
		return -EINVAL;
	}

	act = _ReadRegBits(0, P0REG_IPV6RLTCFG, (header<=OP_IPV6_HEADER_AUTH)?(5-header):(header+1), 1);
	((struct IPv6Setting *)cdata) ->act = act;

	ip1829drv_dbg("cdata ->header=0x%08X\n", ((struct IPv6Setting *)cdata) ->header);
	ip1829drv_dbg("cdata ->act=0x%08X\n", ((struct IPv6Setting *)cdata) ->act);
	FUNC_MSG_OUT;
	return 0;
}

int setCapIpv6ToAllPortsHigh(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _setGeneralEnable(cdata, len, 0, P0REG_IPV6RLTCFG, 8);
	FUNC_MSG_OUT;
	return ret;
}

int getCapIpv6ToAllPortsHigh(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _getGeneralEnable(cdata, len, 0, P0REG_IPV6RLTCFG, 8);
	FUNC_MSG_OUT;
	return ret;
}

int setCapIpv6ToCpuHigh(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _setGeneralEnable(cdata, len, 0, P0REG_IPV6RLTCFG, 9);
	FUNC_MSG_OUT;
	return ret;
}

int getCapIpv6ToCpuHigh(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _getGeneralEnable(cdata, len, 0, P0REG_IPV6RLTCFG, 9);
	FUNC_MSG_OUT;
	return ret;
}

int setCapIpv6Act(void *cdata, int len)
{
	unsigned int header;
	int act;

	FUNC_MSG_IN;
	if (sizeof(struct IPv6Setting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	header = ((struct IPv6Setting *)cdata) ->header;
	if( header >=  OP_IPV6_HEADER_TOTALNUM )
	{
		ip1829drv_dbg("Error: header=%X\n", header);
		return -EINVAL;
	}

	act = ((struct IPv6Setting *)cdata) ->act;
	if( act < OP_CAP_ACT_FORWARD || act > OP_CAP_ACT_DROP )
	{
		ip1829drv_dbg("Error: act=%X\n", act);
		return -EINVAL;
	}

	ip1829drv_dbg("cdata ->header=%X\n", header);
	ip1829drv_dbg("cdata ->act=%X\n", act);

	_WriteRegBits(0, P0REG_IPV6RLTFWD + header/8, (header%8)*2, 2, act);

	FUNC_MSG_OUT;
	return 0;
}

int getCapIpv6Act(void *cdata, int len)
{
	unsigned int header;
	int act;

	FUNC_MSG_IN;
	if (sizeof(struct IPv6Setting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	header = ((struct IPv6Setting *)cdata) ->header;
	if( header >=  OP_IPV6_HEADER_TOTALNUM )
	{
		ip1829drv_dbg("Error: header=%X\n", header);
		return -EINVAL;
	}

	act = _ReadRegBits(0, P0REG_IPV6RLTFWD + header/8, (header%8)*2, 2);
	((struct IPv6Setting *)cdata) ->act = act;

	ip1829drv_dbg("cdata ->header=0x%08X\n", ((struct IPv6Setting *)cdata) ->header);
	ip1829drv_dbg("cdata ->act=0x%08X\n", ((struct IPv6Setting *)cdata) ->act);
	FUNC_MSG_OUT;
	return 0;
}

int setCapIpv6User(void *cdata, int len)
{
	unsigned int header;
	int act;

	FUNC_MSG_IN;
	if (sizeof(struct IPv6Setting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	header = ((struct IPv6Setting *)cdata) ->header;
	if( header != OP_IPV6_HEADER_USER1 && header != OP_IPV6_HEADER_USER2 )
	{
		ip1829drv_dbg("Error: header=%X\n", header);
		return -EINVAL;
	}

	act = ((struct IPv6Setting *)cdata) ->act;
	if( act < 0x0 || act > 0xFF )
	{
		ip1829drv_dbg("Error: act=%X\n", act);
		return -EINVAL;
	}

	ip1829drv_dbg("cdata ->header=%X\n", header);
	ip1829drv_dbg("cdata ->act=%X\n", act);

	_WriteRegBits(0, P0REG_IPV6NXHEADUSER, (header==OP_IPV6_HEADER_USER1?0:8), 8, act);

	FUNC_MSG_OUT;
	return 0;
}

int getCapIpv6User(void *cdata, int len)
{
	unsigned int header;
	int act;

	FUNC_MSG_IN;
	if (sizeof(struct IPv6Setting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	header = ((struct IPv6Setting *)cdata) ->header;
	if( header != OP_IPV6_HEADER_USER1 && header != OP_IPV6_HEADER_USER2 )
	{
		ip1829drv_dbg("Error: header=%X\n", header);
		return -EINVAL;
	}

	act = _ReadRegBits(0, P0REG_IPV6NXHEADUSER, (header==OP_IPV6_HEADER_USER1?0:8), 8);
	((struct IPv6Setting *)cdata) ->act = act;

	ip1829drv_dbg("cdata ->header=0x%08X\n", ((struct IPv6Setting *)cdata) ->header);
	ip1829drv_dbg("cdata ->act=0x%08X\n", ((struct IPv6Setting *)cdata) ->act);
	FUNC_MSG_OUT;
	return 0;
}

int setCapIcmpv6User(void *cdata, int len)
{
	unsigned int header;
	int act;

	FUNC_MSG_IN;
	if (sizeof(struct IPv6Setting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	header = ((struct IPv6Setting *)cdata) ->header;
	if( header > OP_IPV6_HEADER_ICMPV6_USER2_LOW )
	{
		ip1829drv_dbg("Error: header=%X\n", header);
		return -EINVAL;
	}

	act = ((struct IPv6Setting *)cdata) ->act;
	if( act < 0x0 || act > 0xFF )
	{
		ip1829drv_dbg("Error: act=%X\n", act);
		return -EINVAL;
	}

	ip1829drv_dbg("cdata ->header=%X\n", header);
	ip1829drv_dbg("cdata ->act=%X\n", act);

	_WriteRegBits(0, P0REG_ICMPV6TYPEUSER + header/2, (header%2)*8, 8, act);

	FUNC_MSG_OUT;
	return 0;
}

int getCapIcmpv6User(void *cdata, int len)
{
	unsigned int header;
	int act;

	FUNC_MSG_IN;
	if (sizeof(struct IPv6Setting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	header = ((struct IPv6Setting *)cdata) ->header;
	if( header > OP_IPV6_HEADER_ICMPV6_USER2_LOW )
	{
		ip1829drv_dbg("Error: header=%X\n", header);
		return -EINVAL;
	}

	act = _ReadRegBits(0, P0REG_ICMPV6TYPEUSER + header/2, (header%2)*8, 8);
	((struct IPv6Setting *)cdata) ->act = act;

	ip1829drv_dbg("cdata ->header=0x%08X\n", ((struct IPv6Setting *)cdata) ->header);
	ip1829drv_dbg("cdata ->act=0x%08X\n", ((struct IPv6Setting *)cdata) ->act);
	FUNC_MSG_OUT;
	return 0;
}

//------------ Capture functions:common end  -----------------------

int setSMACLearning(void *cdata, int len)
{
	int port, en;
	u32 u32dat;

	ip1829drv_dbg("ip1829: +setSMACLearning...\n");
	if (sizeof(struct ByPortSetting) != len)
		return -EINVAL;

	port=((struct ByPortSetting *)cdata) ->port;
	en = ((struct ByPortSetting *)cdata) ->pdata;
	if (port < 1 || port > SWITCH_MAX_IPORT_CNT)
		return -EINVAL;
	if ( !(en==OP_FUNC_DISABLE || en==OP_FUNC_ENABLE) )
		return -EINVAL;
	ip1829drv_dbg("port=%d\n", port);
	ip1829drv_dbg("en=%d\n", en);

	IP2Page(1);
	u32dat = (u32)( (Read_Reg(P1REG_SRCLEARNEN+1) << 16) | Read_Reg(P1REG_SRCLEARNEN) );
	if (en == OP_FUNC_ENABLE)
		u32dat |= (u32)(1 << (port-1));
	else
		u32dat &= (u32)~(1 << (port-1));
	Write_Reg(P1REG_SRCLEARNEN, u32dat & 0xFFFF);
	Write_Reg(P1REG_SRCLEARNEN+1, u32dat >> 16);

	ip1829drv_dbg("ip1829: -setSMACLearning...\n");
	return 0;
}

int getSMACLearning(void *cdata, int len)
{
	int port, en;

	ip1829drv_dbg("ip1829: +getSMACLearning...\n");
	if (sizeof(struct ByPortSetting) != len)
		return -EINVAL;

	port = ((struct ByPortSetting *)cdata) ->port;
	if (port < 1 || port > SWITCH_MAX_IPORT_CNT)
		return -EINVAL;
	ip1829drv_dbg("port=%d\n", port);

	en = (int)_ReadRegBits(1, P1REG_SRCLEARNEN + (port-1)/16, (port-1)%16, 1);
	((struct ByPortSetting *)cdata) ->pdata = en;
	ip1829drv_dbg("cdata ->pdata=%d\n", ((struct ByPortSetting *)cdata) ->pdata);
	ip1829drv_dbg("ip1829: -getSMACLearning...\n");
	return 0;
}

int setSMACLrnCntCtrl(void *cdata, int len)
{
	int port, en;
	u32 u32dat;

	ip1829drv_dbg("ip1829: +setSMACLrnCntCtrl...\n");
	if (sizeof(struct ByPortSetting) != len)
		return -EINVAL;

	port=((struct ByPortSetting *)cdata) ->port;
	en = ((struct ByPortSetting *)cdata) ->pdata;
	if (port < 1 || port > SWITCH_MAX_IPORT_CNT)
		return -EINVAL;
	if ( !(en==OP_FUNC_DISABLE || en==OP_FUNC_ENABLE) )
		return -EINVAL;
	ip1829drv_dbg("port=%d\n", port);
	ip1829drv_dbg("en=%d\n", en);

	IP2Page(1);
	u32dat = (u32)( (Read_Reg(P1REG_SRCLEARNCNT+1) << 16) | Read_Reg(P1REG_SRCLEARNCNT) );
	if (en == OP_FUNC_ENABLE)
		u32dat |= (u32)(1 << (port-1));
	else
		u32dat &= (u32)~(1 << (port-1));
	Write_Reg(P1REG_SRCLEARNCNT, u32dat & 0xFFFF);
	Write_Reg(P1REG_SRCLEARNCNT+1, u32dat >> 16);

	ip1829drv_dbg("ip1829: -setSMACLrnCntCtrl...\n");
	return 0;
}

int getSMACLrnCntCtrl(void *cdata, int len)
{
	int port, en;

	ip1829drv_dbg("ip1829: +getSMACLrnCntCtrl...\n");
	if (sizeof(struct ByPortSetting) != len)
		return -EINVAL;

	port = ((struct ByPortSetting *)cdata) ->port;
	if (port < 1 || port > SWITCH_MAX_IPORT_CNT)
		return -EINVAL;
	ip1829drv_dbg("port=%d\n", port);

	en = (int)_ReadRegBits(1, P1REG_SRCLEARNCNT + (port-1)/16, (port-1)%16, 1);
	((struct ByPortSetting *)cdata) ->pdata = en;
	ip1829drv_dbg("cdata ->pdata=%d\n", ((struct ByPortSetting *)cdata) ->pdata);
	ip1829drv_dbg("ip1829: -getSMACLrnCntCtrl...\n");
	return 0;
}

int setSMACLrnThreshold(void *cdata, int len)
{
	int threshold;

	ip1829drv_dbg("ip1829: +setSMACLrnThreshold...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	threshold = ((struct GeneralSetting *)cdata) ->gdata;
	if (threshold < 0 || threshold > 0x1FF)
		return -EINVAL;
	ip1829drv_dbg("threshold=%d\n", threshold);

	IP2Page(1);
	Write_Reg(P1REG_SRCLCNTTHRESH, (u16)threshold);

	ip1829drv_dbg("ip1829: -setSMACLrnThreshold...\n");
	return 0;
}

int getSMACLrnThreshold(void *cdata, int len)
{

	ip1829drv_dbg("ip1829: +getSMACLrnThreshold...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	IP2Page(1);
	((struct GeneralSetting *)cdata) ->gdata = (int)Read_Reg(P1REG_SRCLCNTTHRESH);
	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	ip1829drv_dbg("ip1829: -getSMACLrnThreshold...\n");
	return 0;
}

int setLutPortFlush(void *cdata, int len)
{
	unsigned long pm;
	int en;
	u32 u32dat;
	int while_cnt = 0;
	int reg = 0;

	ip1829drv_dbg("ip1829: +setLutPortFlush...\n");
	if (sizeof(struct PortmapSetting) != len)
		return -EINVAL;

	pm = ((struct PortmapSetting *)cdata) ->portmap;
	en = ((struct PortmapSetting *)cdata) ->pmdata;
	if (pm & 0xE0000000)
		return -EINVAL;
	if (en != OP_FUNC_ENABLE)
		return -EINVAL;
	ip1829drv_dbg("pm=0x%08x\n", (u32)pm);
	ip1829drv_dbg("en=%d\n", en);

	IP2Page(1);
	u32dat = pm | (u32)0x20000000;
	Write_Reg(P1REG_PORTFLUSH, (u16)(u32dat & 0xFFFF));
	Write_Reg(P1REG_PORTFLUSH+1, (u16)(u32dat >> 16));
	while(1)
	{
		reg = Read_Reg(P1REG_PORTFLUSH+1);
		if (((reg >> 13)&0x1) == 0)
			break;
		while_cnt ++;
		if (while_cnt > 10000)
		{
			printk ("setLutPortFlush Fail\n");
			return -EINVAL;
		}
	}

	ip1829drv_dbg("ip1829: -setLutPortFlush...\n");
	return 0;
}

int setSnifferSrc(void *cdata, int len)
{
	unsigned long pm;

	ip1829drv_dbg("ip1829: +setSnifferSrc...\n");
	if (sizeof(struct PortMemberSetting) != len)
		return -EINVAL;

	pm = ((struct PortMemberSetting *)cdata) ->member;
	if (pm & 0xE0000000)
		return -EINVAL;
	ip1829drv_dbg("pm=0x%08x\n",(unsigned int)pm);

	IP2Page(1);
	Write_Reg(P1REG_SNIFSRC, (u16)(pm & 0xFFFF));
	Write_Reg(P1REG_SNIFSRC+1, (u16)(pm >> 16));

	ip1829drv_dbg("ip1829: -setSnifferSrc...\n");
	return 0;
}

int getSnifferSrc(void *cdata, int len)
{
	u32 u32dat;

	ip1829drv_dbg("ip1829: +getSnifferSrc...\n");
	if (sizeof(struct PortMemberSetting) != len)
		return -EINVAL;

	IP2Page(1);
	u32dat = (u32)( (Read_Reg(P1REG_SNIFSRC+1) << 16) | Read_Reg(P1REG_SNIFSRC) );
	((struct PortMemberSetting *)cdata) ->member = (u32dat & 0x1FFFFFFF);
	ip1829drv_dbg("cdata ->gdata=0x%08x\n",(unsigned int)((struct PortMemberSetting *)cdata) ->member);
	ip1829drv_dbg("ip1829: -getSnifferSrc...\n");
	return 0;
}

int setSnifferDestGrp1(void *cdata, int len)
{
	unsigned long pm;
	u32 u32dat;

	ip1829drv_dbg("ip1829: +setSnifferDestGrp1...\n");
	if (sizeof(struct PortMemberSetting) != len)
		return -EINVAL;

	pm = ((struct PortMemberSetting *)cdata) ->member;
	if (pm & 0xE0000000)
		return -EINVAL;
	ip1829drv_dbg("pm=0x%08x\n", (unsigned int)pm);

	IP2Page(1);
	u32dat = (u32)( (Read_Reg(P1REG_SNIFDEST1) << 16) | Read_Reg(P1REG_SNIFDEST) );
	u32dat = ((u32dat & 0xE0000000) | pm);
	Write_Reg(P1REG_SNIFDEST, (u16)(u32dat & 0xFFFF));
	Write_Reg(P1REG_SNIFDEST1, (u16)(u32dat >> 16));

	ip1829drv_dbg("ip1829: -setSnifferDestGrp1...\n");
	return 0;
}

int getSnifferDestGrp1(void *cdata, int len)
{
	u32 u32dat;

	ip1829drv_dbg("ip1829: +getSnifferDestGrp1...\n");
	if (sizeof(struct PortMemberSetting) != len)
		return -EINVAL;

	IP2Page(1);
	u32dat = (u32)( (Read_Reg(P1REG_SNIFDEST1) << 16) | Read_Reg(P1REG_SNIFDEST) );
	((struct PortMemberSetting *)cdata) ->member = (u32dat & 0x1FFFFFFF);
	ip1829drv_dbg("cdata ->gdata=0x%08x\n", (unsigned int)((struct PortMemberSetting *)cdata) ->member);
	ip1829drv_dbg("ip1829: -getSnifferDestGrp1...\n");
	return 0;
}

int setSnifferDestGrp2(void *cdata, int len)
{
	unsigned long pm;
	u32 u32dat;

	ip1829drv_dbg("ip1829: +setSnifferDestGrp2...\n");
	if (sizeof(struct PortMemberSetting) != len)
		return -EINVAL;

	pm = ((struct PortMemberSetting *)cdata) ->member;
	if (pm & 0xE0000000)
		return -EINVAL;
	ip1829drv_dbg("pm=0x%08x\n", (unsigned int)pm);

	IP2Page(1);
	u32dat = (u32)( (Read_Reg(P1REG_SNIF2DEST1) << 16) | Read_Reg(P1REG_SNIF2DEST) );
	u32dat = ((u32dat & 0xE0000000) | pm);
	Write_Reg(P1REG_SNIF2DEST, (u16)(u32dat & 0xFFFF));
	Write_Reg(P1REG_SNIF2DEST1, (u16)(u32dat >> 16));

	ip1829drv_dbg("ip1829: -setSnifferDestGrp2...\n");
	return 0;
}

int getSnifferDestGrp2(void *cdata, int len)
{
	u32 u32dat;

	ip1829drv_dbg("ip1829: +getSnifferDestGrp2...\n");
	if (sizeof(struct PortMemberSetting) != len)
		return -EINVAL;

	IP2Page(1);
	u32dat = (u32)( (Read_Reg(P1REG_SNIF2DEST1) << 16) | Read_Reg(P1REG_SNIF2DEST) );
	((struct PortMemberSetting *)cdata) ->member = (u32dat & 0x1FFFFFFF);
	ip1829drv_dbg("cdata ->gdata=0x%08x\n", (unsigned int)((struct PortMemberSetting *)cdata) ->member);
	ip1829drv_dbg("ip1829: -getSnifferDestGrp2...\n");
	return 0;
}

int setS1Method(void *cdata, int len)
{
	int method;

	ip1829drv_dbg("ip1829: +setS1Method...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	method = ((struct GeneralSetting *)cdata) ->gdata;
	if (method<OP_SNIFFER1_METHOD_DISABLE || method>OP_SNIFFER1_METHOD_BOTHDIR)
		return -EINVAL;
	ip1829drv_dbg("method=%d\n", method);

	_WriteRegBits(1, P1REG_SNIFDEST1, 13, 2, method);
	ip1829drv_dbg("ip1829: -setS1Method...\n");
	return 0;
}

int getS1Method(void *cdata, int len)
{
	ip1829drv_dbg("ip1829: +getS1Method...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	((struct GeneralSetting *)cdata) ->gdata = (int)_ReadRegBits(1, P1REG_SNIFDEST1,13, 2);
	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	ip1829drv_dbg("ip1829: -getS1Method...\n");
	return 0;
}

int setStormFunc(void *cdata, int len)
{
	u8 storm, u8dat=0;
	long porten;

	ip1829drv_dbg("ip1829: +setStormFunc...\n");
	if (sizeof(struct StormGeneralSetting) != len)
		return -EINVAL;

	storm = ((struct StormGeneralSetting *)cdata) ->storm;
	porten= ((struct StormGeneralSetting *)cdata) ->sdata;
	if (storm!=OP_STORM_BCST && storm!=OP_STORM_MCST && storm!=OP_STORM_DLF &&
			storm!=OP_STORM_ARP && storm!=OP_STORM_ICMP)
		return -EINVAL;
	if (porten<0 || (porten&0xE0000000))
		return -EINVAL;
	ip1829drv_dbg("storm=0x%02X\n", storm);
	ip1829drv_dbg("porten=0x%08x\n", (unsigned int)porten);

	IP2Page(1);
	switch (storm)
	{
		case OP_STORM_BCST:
			u8dat = P1REG_BSTORMEN;		break;

		case OP_STORM_MCST:
			u8dat = P1REG_MSTORMEN;		break;

		case OP_STORM_DLF:
			u8dat = P1REG_DLFSTORMEN;	break;

		case OP_STORM_ARP:
			u8dat = P1REG_ARPSTORMEN;	break;

		case OP_STORM_ICMP:
			u8dat = P1REG_ICMPSTORMEN;	break;
	}
	Write_Reg(u8dat, (u16)(porten & 0xFFFF));
	Write_Reg(u8dat+1, (u16)(porten >> 16));

	ip1829drv_dbg("ip1829: -setStormFunc...\n");
	return 0;
}

int getStormFunc(void *cdata, int len)
{
	u8 storm, u8dat=0;
	u32 u32dat;

	ip1829drv_dbg("ip1829: +getStormFunc...\n");
	if (sizeof(struct StormGeneralSetting) != len)
		return -EINVAL;

	storm = ((struct StormGeneralSetting *)cdata) ->storm;
	if (storm!=OP_STORM_BCST && storm!=OP_STORM_MCST && storm!=OP_STORM_DLF &&
			storm!=OP_STORM_ARP && storm!=OP_STORM_ICMP)
		return -EINVAL;
	ip1829drv_dbg("storm=0x%02X\n", storm);

	IP2Page(1);
	switch (storm)
	{
		case OP_STORM_BCST:
			u8dat = P1REG_BSTORMEN;		break;

		case OP_STORM_MCST:
			u8dat = P1REG_MSTORMEN;		break;

		case OP_STORM_DLF:
			u8dat = P1REG_DLFSTORMEN;	break;

		case OP_STORM_ARP:
			u8dat = P1REG_ARPSTORMEN;	break;

		case OP_STORM_ICMP:
			u8dat = P1REG_ICMPSTORMEN;	break;
	}
	u32dat = (u32)( (Read_Reg(u8dat+1) << 16) | Read_Reg(u8dat) );
	((struct StormGeneralSetting *)cdata) ->sdata = (long)u32dat;
	ip1829drv_dbg("cdata ->sdata=0x%08x\n", (unsigned int)((struct StormGeneralSetting *)cdata) ->sdata);
	ip1829drv_dbg("ip1829: -getStormFunc...\n");
	return 0;
}

int setStormThreshold(void *cdata, int len)
{
	u8 storm, u8dat=0;
	long threshold;

	ip1829drv_dbg("ip1829: +setStormThreshold...\n");
	if (sizeof(struct StormGeneralSetting) != len)
		return -EINVAL;

	storm = ((struct StormGeneralSetting *)cdata) ->storm;
	threshold = ((struct StormGeneralSetting *)cdata) ->sdata;
	if (storm!=OP_STORM_BCST && storm!=OP_STORM_MCST && storm!=OP_STORM_DLF &&
			storm!=OP_STORM_ARP && storm!=OP_STORM_ICMP)
		return -EINVAL;
	if (threshold<0 || threshold>0xFF)
		return -EINVAL;
	ip1829drv_dbg("storm=0x%02X\n", storm);
	ip1829drv_dbg("threshold=0x%08x\n", (unsigned int)threshold);

	switch (storm)
	{
		case OP_STORM_BCST:
		case OP_STORM_MCST:
		case OP_STORM_DLF:
			u8dat = P1REG_BSTORMTHRESH;	break;

		case OP_STORM_ARP:
			u8dat = P1REG_ARPSTORMCFG;	break;

		case OP_STORM_ICMP:
			u8dat = P1REG_ICMPSTORMCFG;	break;
	}
	_WriteRegBits(1, u8dat, 0, 8, (u16)threshold);
	ip1829drv_dbg("ip1829: -setStormThreshold...\n");
	return 0;
}

int getStormThreshold(void *cdata, int len)
{
	u8 storm, u8dat=0;

	ip1829drv_dbg("ip1829: +getStormThreshold...\n");
	if (sizeof(struct StormGeneralSetting) != len)
		return -EINVAL;

	storm = ((struct StormGeneralSetting *)cdata) ->storm;
	if (storm!=OP_STORM_BCST && storm!=OP_STORM_MCST && storm!=OP_STORM_DLF &&
			storm!=OP_STORM_ARP && storm!=OP_STORM_ICMP)
		return -EINVAL;
	ip1829drv_dbg("storm=0x%02X\n", storm);

	switch (storm)
	{
		case OP_STORM_BCST:
		case OP_STORM_MCST:
		case OP_STORM_DLF:
			u8dat = P1REG_BSTORMTHRESH;	break;

		case OP_STORM_ARP:
			u8dat = P1REG_ARPSTORMCFG;	break;

		case OP_STORM_ICMP:
			u8dat = P1REG_ICMPSTORMCFG;	break;
	}
	((struct StormGeneralSetting *)cdata) ->sdata = (long)_ReadRegBits(1, u8dat, 0, 8);
	ip1829drv_dbg("cdata ->sdata=0x%08x\n", (unsigned int)((struct StormGeneralSetting *)cdata) ->sdata);
	ip1829drv_dbg("ip1829: -getStormThreshold...\n");
	return 0;
}

int setStormCntrClrPeriod(void *cdata, int len)
{
	u8 storm, reg = 0;
	long period;

	FUNC_MSG_IN;
	if (sizeof(struct StormGeneralSetting) != len)
		return -EINVAL;

	storm = ((struct StormGeneralSetting *)cdata) ->storm;
	period = ((struct StormGeneralSetting *)cdata) ->sdata;
	if (		storm!=OP_STORM_BCST
			&&	storm!=OP_STORM_MCST
			&&	storm!=OP_STORM_DLF
			&&	storm!=OP_STORM_ARP
			&&	storm!=OP_STORM_ICMP)
		return -EINVAL;
	if (period<0 || period>3)
		return -EINVAL;
	ip1829drv_dbg("storm=0x%02X\n", storm);
	ip1829drv_dbg("period=0x%X\n", (unsigned int)period);

	switch (storm)
	{
		case OP_STORM_BCST:
		case OP_STORM_MCST:
		case OP_STORM_DLF:
			reg = P1REG_BSTORMTHRESH;
			break;
		case OP_STORM_ARP:
			reg = P1REG_ARPSTORMCFG;
			break;
		case OP_STORM_ICMP:
			reg = P1REG_ICMPSTORMCFG;
			break;
	}
	ip1829drv_dbg("cdata ->sdata=0x%X\n", (s16)period);
	_WriteRegBits(1, reg, 8, 2, period);

	FUNC_MSG_OUT;
	return 0;

}

int getStormCntrClrPeriod(void *cdata, int len)
{
	u8 storm, reg=0;
	long period;

	FUNC_MSG_IN;
	if (sizeof(struct StormGeneralSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	storm = ((struct StormGeneralSetting *)cdata) ->storm;
	ip1829drv_dbg("storm=0x%02X\n", storm);
	switch (storm)
	{
		case OP_STORM_BCST:
		case OP_STORM_MCST:
		case OP_STORM_DLF:
			reg = P1REG_BSTORMTHRESH;
			break;
		case OP_STORM_ARP:
			reg = P1REG_ARPSTORMCFG;
			break;
		case OP_STORM_ICMP:
			reg = P1REG_ICMPSTORMCFG;
			break;
	}

	period = _ReadRegBits(1, reg, 8, 2);
	((struct StormGeneralSetting *)cdata) ->sdata = period;

	ip1829drv_dbg("cdata ->sdata=0x%X\n", (s16)period);
	FUNC_MSG_OUT;
	return 0;
}

int setStormBlockFrm2Cpu(void *cdata, int len)
{
	u8 storm, u8dat=0;
	long en;

	ip1829drv_dbg("ip1829: +setStormBlockFrm2Cpu...\n");
	if (sizeof(struct StormGeneralSetting) != len)
		return -EINVAL;

	storm = ((struct StormGeneralSetting *)cdata) ->storm;
	en = ((struct StormGeneralSetting *)cdata) ->sdata;
	if (storm!=OP_STORM_BCST && storm!=OP_STORM_MCST && storm!=OP_STORM_ARP)
		return -EINVAL;
	if (en!=OP_FUNC_DISABLE && en!=OP_FUNC_ENABLE)
		return -EINVAL;
	ip1829drv_dbg("storm=0x%02X\n", storm);
	ip1829drv_dbg("en=0x%08x\n", (unsigned int)en);

	switch (storm)
	{
		case OP_STORM_BCST:
		case OP_STORM_MCST:
			u8dat = P1REG_BSTORMTHRESH;	break;

		case OP_STORM_ARP:
			u8dat = P1REG_ARPSTORMCFG;	break;
	}
	_WriteRegBits(1, u8dat, 10, 1, en);


	ip1829drv_dbg("ip1829: -setStormBlockFrm2Cpu...\n");
	return 0;
}

int getStormBlockFrm2Cpu(void *cdata, int len)
{
	u8 storm, u8dat=0;

	ip1829drv_dbg("ip1829: +getStormBlockFrm2Cpu...\n");
	if (sizeof(struct StormGeneralSetting) != len)
		return -EINVAL;

	storm = ((struct StormGeneralSetting *)cdata) ->storm;
	if (storm!=OP_STORM_BCST && storm!=OP_STORM_MCST && storm!=OP_STORM_ARP)
		return -EINVAL;
	ip1829drv_dbg("storm=0x%02X\n", storm);

	switch (storm)
	{
		case OP_STORM_BCST:
		case OP_STORM_MCST:
			u8dat = P1REG_BSTORMTHRESH;	break;

		case OP_STORM_ARP:
			u8dat = P1REG_ARPSTORMCFG;	break;
	}
	((struct StormGeneralSetting *)cdata) ->sdata = (long)_ReadRegBits(1, u8dat,10, 1);
	ip1829drv_dbg("cdata ->sdata=0x%08x\n", (unsigned int)((struct StormGeneralSetting *)cdata) ->sdata);
	ip1829drv_dbg("ip1829: -getStormBlockFrm2Cpu...\n");
	return 0;
}

int setStormDropInterrupt(void *cdata, int len)
{
	u8 storm;
	long en;

	ip1829drv_dbg("ip1829: +setStormDropInterrupt...\n");
	if (sizeof(struct StormGeneralSetting) != len)
		return -EINVAL;

	storm = ((struct StormGeneralSetting *)cdata) ->storm;
	en = ((struct StormGeneralSetting *)cdata) ->sdata;
	if (storm!=OP_STORM_ARP && storm!=OP_STORM_ICMP)
		return -EINVAL;
	if (en!=OP_FUNC_DISABLE && en!=OP_FUNC_ENABLE)
		return -EINVAL;
	ip1829drv_dbg("storm=0x%02X\n", storm);
	ip1829drv_dbg("en=0x%08x\n", (unsigned int)en);

	switch (storm)
	{
		case OP_STORM_ARP:
			_WriteRegBits(1, P1REG_ARPSTORMCFG, 11, 1, (en == OP_FUNC_DISABLE)?0:1);
			break;

		case OP_STORM_ICMP:
			_WriteRegBits(1, P1REG_ICMPSTORMCFG, 10, 1, (en == OP_FUNC_DISABLE)?0:1);
			break;
	}

	ip1829drv_dbg("ip1829: -setStormDropInterrupt...\n");
	return 0;
}

int getStormDropInterrupt(void *cdata, int len)
{
	u8 storm, u8dat=0;

	ip1829drv_dbg("ip1829: +getStormDropInterrupt...\n");
	if (sizeof(struct StormGeneralSetting) != len)
		return -EINVAL;

	storm = ((struct StormGeneralSetting *)cdata) ->storm;
	if (storm!=OP_STORM_ARP && storm!=OP_STORM_ICMP)
		return -EINVAL;
	ip1829drv_dbg("storm=0x%02X\n", storm);

	switch (storm)
	{
		case OP_STORM_ARP:
			((struct StormGeneralSetting *)cdata) ->sdata = (long)_ReadRegBits(1, u8dat, 11, 1);
			break;

		case OP_STORM_ICMP:
			((struct StormGeneralSetting *)cdata) ->sdata = (long)_ReadRegBits(1, u8dat, 10, 1);
			break;
	}
	ip1829drv_dbg("cdata ->sdata=0x%08x\n", (unsigned int)((struct StormGeneralSetting *)cdata) ->sdata);
	ip1829drv_dbg("ip1829: -getStormDropInterrupt...\n");
	return 0;
}

int setEocFunc(void *cdata, int len)
{
	unsigned long pm;
	int en;
	u32 u32dat;

	ip1829drv_dbg("ip1829: +setEocFunc...\n");
	if (sizeof(struct PortmapSetting) != len)
		return -EINVAL;

	pm = ((struct PortmapSetting *)cdata) ->portmap;
	en = ((struct PortmapSetting *)cdata) ->pmdata;
	if (pm & 0xE0000000)
		return -EINVAL;
	if (en != OP_FUNC_ENABLE)
		return -EINVAL;
	ip1829drv_dbg("pm=0x%08x\n", (unsigned int)pm);
	ip1829drv_dbg("en=%d\n", en);

	IP2Page(0);
	u32dat = (u32)( (Read_Reg(P0REG_EOCCFG1) << 16) | Read_Reg(P0REG_EOCCFG0) );
	u32dat = ((u32dat & 0xE0000000) | pm);
	Write_Reg(P0REG_EOCCFG0, (u16)(u32dat & 0xFFFF));
	Write_Reg(P0REG_EOCCFG1, (u16)(u32dat >> 16));

	ip1829drv_dbg("ip1829: -setEocFunc...\n");
	return 0;
}

int getEocFunc(void *cdata, int len)
{
	u32 u32dat;

	ip1829drv_dbg("ip1829: +getEocFunc...\n");
	if (sizeof(struct PortmapSetting) != len)
		return -EINVAL;

	IP2Page(0);
	u32dat = (u32)( (Read_Reg(P0REG_EOCCFG1) << 16) | Read_Reg(P0REG_EOCCFG0) );
	((struct PortmapSetting *)cdata) ->portmap = (u32dat & 0x1FFFFFFF);
	((struct PortmapSetting *)cdata) ->pmdata = OP_FUNC_ENABLE;
	ip1829drv_dbg("cdata ->portmap=0x%08x\n", (unsigned int)((struct PortmapSetting *)cdata) ->portmap);
	ip1829drv_dbg("cdata ->pmdata=%d\n", ((struct PortmapSetting *)cdata) ->pmdata);
	ip1829drv_dbg("ip1829: -getEocFunc...\n");
	return 0;
}

int getEocStatus(void *cdata, int len)
{
	int i;
	u32 u32dat;

	ip1829drv_dbg("ip1829: +getEocStatus...\n");
	if (sizeof(struct AllPortsSetting) != len)
		return -EINVAL;

	IP2Page(0);
	u32dat = (u32)( (Read_Reg(P0REG_EOCBLKSTATUS+1) << 16) | Read_Reg(P0REG_EOCBLKSTATUS) );
	ip1829drv_dbg("cdata ->apdata=\n[");
	for (i=0; i < SWITCH_MAX_IPORT_CNT; i++)
	{
		((struct AllPortsSetting *)cdata) ->apdata[i] = (int)(u32dat & 0x1);
		ip1829drv_dbg("%d, ", ((struct AllPortsSetting *)cdata) ->apdata[i]);
		if (i%5 == 4)
			ip1829drv_dbg("   ");
		if (i%10 == 9)
			ip1829drv_dbg("\n ");
		u32dat = u32dat >> 1;
	}
	ip1829drv_dbg("]\n");
	ip1829drv_dbg("ip1829: -getEocStatus...\n");
	return 0;
}

int setEocReleaseTime(void *cdata, int len)
{
	int rtime;

	ip1829drv_dbg("ip1829: +setEocReleaseTime...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	rtime = ((struct GeneralSetting *)cdata) ->gdata;
	if (rtime!=OP_EOC_RELEASE_TIME_1MIN && rtime!=OP_EOC_RELEASE_TIME_10MIN)
		return -EINVAL;
	ip1829drv_dbg("rtime=%d\n", rtime);

	_WriteRegBits(0, P0REG_EOCCFG1, 14, 1, (rtime == OP_EOC_RELEASE_TIME_1MIN)?0:1);

	ip1829drv_dbg("ip1829: -setEocReleaseTime...\n");
	return 0;
}

int getEocReleaseTime(void *cdata, int len)
{
	int rtime;

	ip1829drv_dbg("ip1829: +getEocReleaseTime...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	rtime = (int)_ReadRegBits(0, P0REG_EOCCFG1, 14, 1);

	if (rtime)
		rtime = OP_EOC_RELEASE_TIME_10MIN;
	else
		rtime = OP_EOC_RELEASE_TIME_1MIN;
	((struct GeneralSetting *)cdata) ->gdata = rtime;
	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	ip1829drv_dbg("ip1829: -getEocReleaseTime...\n");
	return 0;
}

int setLdFunc(void *cdata, int len)
{
	unsigned long pm;
	int en;
	u32 u32dat;

	ip1829drv_dbg("ip1829: +setLdFunc...\n");
	if (sizeof(struct PortmapSetting) != len)
		return -EINVAL;

	pm = ((struct PortmapSetting *)cdata) ->portmap;
	en = ((struct PortmapSetting *)cdata) ->pmdata;
	if (pm & 0xE0000000)
		return -EINVAL;

	ip1829drv_dbg("pm=0x%08x\n", (unsigned int)pm);
	ip1829drv_dbg("en=%d\n", en);
	_WriteRegBits(1, P1REG_MISCCFG, 3, 1, en);

	IP2Page(0);
	u32dat = (u32)( (Read_Reg(P0REG_LDCONFIG1) << 16) | Read_Reg(P0REG_LDCONFIG) );
	u32dat = ((u32dat & 0xE0000000) | pm);
	Write_Reg(P0REG_LDCONFIG, (u16)(u32dat & 0xFFFF));
	Write_Reg(P0REG_LDCONFIG1, (u16)(u32dat >> 16));

	ip1829drv_dbg("ip1829: -setLdFunc...\n");
	return 0;
}

int getLdFunc(void *cdata, int len)
{
	u32 u32dat;
	u16 u16dat;

	ip1829drv_dbg("ip1829: +getLdFunc...\n");
	if (sizeof(struct PortmapSetting) != len)
		return -EINVAL;

	IP2Page(1);
	u16dat = Read_Reg(P1REG_MISCCFG);
	if(u16dat & 0x0008)
		((struct PortmapSetting *)cdata)->pmdata = OP_FUNC_ENABLE;
	else
		((struct PortmapSetting *)cdata)->pmdata = OP_FUNC_DISABLE;

	IP2Page(0);
	u32dat = (u32)( (Read_Reg(P0REG_LDCONFIG1) << 16) | Read_Reg(P0REG_LDCONFIG) );
	((struct PortmapSetting *)cdata) ->portmap = (u32dat & 0x1FFFFFFF);
	ip1829drv_dbg("cdata ->portmap=0x%08x\n", (unsigned int)((struct PortmapSetting *)cdata) ->portmap);
	ip1829drv_dbg("cdata ->pmdata=%d\n", ((struct PortmapSetting *)cdata) ->pmdata);
	ip1829drv_dbg("ip1829: -getLdFunc...\n");
	return 0;
}

int setLdTimeUnit(void *cdata, int len)
{
	int tunit;

	ip1829drv_dbg("ip1829: +setLdTimeUnit...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	tunit = ((struct GeneralSetting *)cdata) ->gdata;
	if (tunit!=OP_LD_TIME_UNIT_500MS && tunit!=OP_LD_TIME_UNIT_1S)
		return -EINVAL;
	ip1829drv_dbg("tunit=%d\n", tunit);

	_WriteRegBits(0, P0REG_LDCONFIG1, 13, 1, tunit);
	ip1829drv_dbg("ip1829: -setLdTimeUnit...\n");
	return 0;
}

int getLdTimeUnit(void *cdata, int len)
{
	ip1829drv_dbg("ip1829: +getLdTimeUnit...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	((struct GeneralSetting *)cdata) ->gdata = (int)_ReadRegBits(0, P0REG_LDCONFIG1, 13, 1);

	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	ip1829drv_dbg("ip1829: -getLdTimeUnit...\n");
	return 0;
}

int setLdPktSendTimer(void *cdata, int len)
{
	int timer;

	ip1829drv_dbg("ip1829: +setLdPktSendTimer...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	timer = ((struct GeneralSetting *)cdata) ->gdata;
	if (timer < 0 || timer > 0xFF)
		return -EINVAL;
	ip1829drv_dbg("timer=%d\n", timer);

	_WriteRegBits(0, P0REG_LDTIMER, 0, 8, timer);
	ip1829drv_dbg("ip1829: -setLdPktSendTimer...\n");
	return 0;
}

int getLdPktSendTimer(void *cdata, int len)
{
	ip1829drv_dbg("ip1829: +getLdPktSendTimer...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	((struct GeneralSetting *)cdata) ->gdata = (int)_ReadRegBits(0, P0REG_LDTIMER, 0, 8);

	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	ip1829drv_dbg("ip1829: -getLdPktSendTimer...\n");
	return 0;
}

int setLdBlockReleaseTimer(void *cdata, int len)
{
	int timer;

	ip1829drv_dbg("ip1829: +setLdBlockReleaseTimer...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	timer = ((struct GeneralSetting *)cdata) ->gdata;
	if (timer < 0 || timer > 0xFF)
		return -EINVAL;
	ip1829drv_dbg("timer=%d\n", timer);

	_WriteRegBits(0, P0REG_LDTIMER, 8, 8, timer);
	ip1829drv_dbg("ip1829: -setLdBlockReleaseTimer...\n");
	return 0;
}

int getLdBlockReleaseTimer(void *cdata, int len)
{
	ip1829drv_dbg("ip1829: +getLdBlockReleaseTimer...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	((struct GeneralSetting *)cdata) ->gdata = (int)_ReadRegBits(0, P0REG_LDTIMER, 8, 8);

	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	ip1829drv_dbg("ip1829: -getLdBlockReleaseTimer...\n");
	return 0;
}

int getLdStatus(void *cdata, int len)
{
	u32 u32dat;

	ip1829drv_dbg("ip1829: +getLdStatus...\n");
	if (sizeof(struct PortMemberSetting) != len)
		return -EINVAL;

	IP2Page(0);
	u32dat = (u32)( (Read_Reg(P0REG_LDSTATUS+1) << 16) | Read_Reg(P0REG_LDSTATUS) );
	((struct PortMemberSetting *)cdata) ->member = (int)u32dat;
	ip1829drv_dbg("cdata ->member=%08lX\n", ((struct PortMemberSetting *)cdata) ->member);
	ip1829drv_dbg("ip1829: -getLdStatus...\n");
	return 0;
}

int setWolFunc(void *cdata, int len)
{
	unsigned long pm;
	int en;

	ip1829drv_dbg("ip1829: +setWolFunc...\n");
	if (sizeof(struct PortmapSetting) != len)
		return -EINVAL;

	pm = ((struct PortmapSetting *)cdata) ->portmap;
	en = ((struct PortmapSetting *)cdata) ->pmdata;
	if (pm & 0xF0000000)
		return -EINVAL;
	if (en != OP_FUNC_ENABLE)
		return -EINVAL;
	ip1829drv_dbg("pm=0x%08x\n", (unsigned int)pm);
	ip1829drv_dbg("en=%d\n", en);

	IP2Page(0);
	Write_Reg(P0REG_WOLEN, (u16)(pm & 0xFFFF));
	Write_Reg(P0REG_WOLEN, (u16)(pm >> 16));

	ip1829drv_dbg("ip1829: -setWolFunc...\n");
	return 0;
}

int getWolFunc(void *cdata, int len)
{
	u32 u32dat;

	ip1829drv_dbg("ip1829: +getWolFunc...\n");
	if (sizeof(struct PortmapSetting) != len)
		return -EINVAL;

	IP2Page(0);
	u32dat = (u32)( (Read_Reg(P0REG_WOLEN+1) << 16) | Read_Reg(P0REG_WOLEN) );
	((struct PortmapSetting *)cdata) ->portmap = (u32dat & ALL_PHY_PORTS_LIST);
	((struct PortmapSetting *)cdata) ->pmdata = OP_FUNC_ENABLE;
	ip1829drv_dbg("cdata ->portmap=0x%08x\n", (unsigned int)((struct PortmapSetting *)cdata) ->portmap);
	ip1829drv_dbg("cdata ->pmdata=%d\n", ((struct PortmapSetting *)cdata) ->pmdata);
	ip1829drv_dbg("ip1829: -getWolFunc...\n");
	return 0;
}

int setWolMode(void *cdata, int len)
{
	int wmode;

	ip1829drv_dbg("ip1829: +setWolMode...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	wmode = ((struct GeneralSetting *)cdata) ->gdata;
	if (wmode!=OP_WOL_MODE_SLAVE && wmode!=OP_WOL_MODE_MASTER)
		return -EINVAL;
	ip1829drv_dbg("wmode=%d\n", wmode);

	_WriteRegBits(0, P0REG_WOLCFG, 9, 1, wmode);
	ip1829drv_dbg("ip1829: -setWolMode...\n");
	return 0;
}

int getWolMode(void *cdata, int len)
{
	ip1829drv_dbg("ip1829: +getWolMode...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	((struct GeneralSetting *)cdata) ->gdata = (int)_ReadRegBits(0, P0REG_WOLCFG, 9, 1);

	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	ip1829drv_dbg("ip1829: -getWolMode...\n");
	return 0;
}

int setWolInterrupt(void *cdata, int len)
{
	int interrupt;

	ip1829drv_dbg("ip1829: +setWolInterrupt...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	interrupt = ((struct GeneralSetting *)cdata) ->gdata;
	if (interrupt!=OP_FUNC_DISABLE && interrupt!=OP_FUNC_ENABLE)
		return -EINVAL;
	ip1829drv_dbg("interrupt=%d\n", interrupt);

	_WriteRegBits(0, P0REG_WOLCFG, 11, 1, interrupt);
	ip1829drv_dbg("ip1829: -setWolInterrupt...\n");
	return 0;
}

int getWolInterrupt(void *cdata, int len)
{
	ip1829drv_dbg("ip1829: +getWolInterrupt...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	((struct GeneralSetting *)cdata) ->gdata = (int)_ReadRegBits(0, P0REG_WOLCFG, 11, 1);

	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	ip1829drv_dbg("ip1829: -getWolInterrupt...\n");
	return 0;
}

int setWolIPUnit(void *cdata, int len)
{
	int unit;

	ip1829drv_dbg("ip1829: +setWolIPUnit...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	unit = ((struct GeneralSetting *)cdata) ->gdata;
	if (unit<OP_WOL_IDLE_UNIT_DISABLE || unit>OP_WOL_IDLE_UNIT_10min)
		return -EINVAL;
	ip1829drv_dbg("unit=%d\n", unit);

	_WriteRegBits(0, P0REG_WOLCFG, 5, 2, unit);
	ip1829drv_dbg("ip1829: -setWolIPUnit...\n");
	return 0;
}

int getWolIPUnit(void *cdata, int len)
{
	ip1829drv_dbg("ip1829: +getWolIPUnit...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	((struct GeneralSetting *)cdata) ->gdata = (int)_ReadRegBits(0, P0REG_WOLCFG, 5, 2);

	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	ip1829drv_dbg("ip1829: -getWolIPUnit...\n");
	return 0;
}

int setWolIPThreshold(void *cdata, int len)
{
	int threshold;

	ip1829drv_dbg("ip1829: +setWolIPThreshold...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	threshold = ((struct GeneralSetting *)cdata) ->gdata;
	if (threshold<1 || threshold>32)
		return -EINVAL;
	ip1829drv_dbg("threshold=%d\n", threshold);

	_WriteRegBits(0, P0REG_WOLCFG, 0, 5, (u16)(threshold - 1));
	ip1829drv_dbg("ip1829: -setWolIPThreshold...\n");
	return 0;
}

int getWolIPThreshold(void *cdata, int len)
{
	ip1829drv_dbg("ip1829: +getWolIPThreshold...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	((struct GeneralSetting *)cdata) ->gdata = (int)_ReadRegBits(0, P0REG_WOLCFG, 0, 5)+1;

	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	ip1829drv_dbg("ip1829: -getWolIPThreshold...\n");
	return 0;
}

int setWolStatusInSlaveMode(void *cdata, int len)
{
	int port, state;
	u32 u32dat;

	ip1829drv_dbg("ip1829: +setWolStatusInSlaveMode...\n");
	if (sizeof(struct ByPortSetting) != len)
		return -EINVAL;

	port = ((struct ByPortSetting *)cdata) ->port;
	state= ((struct ByPortSetting *)cdata) ->pdata;
	if (port < 1 || port > (SWITCH_MAX_IPORT_CNT-1))
		return -EINVAL;
	if (state != OP_WOL_STATUS_SLEEP)
		return -EINVAL;
	ip1829drv_dbg("port=%d\n", port);
	ip1829drv_dbg("state=%d\n", state);

	IP2Page(0);
	u32dat = (u32)( (Read_Reg(P0REG_WOLMODECTRL+1) << 16) | Read_Reg(P0REG_WOLMODECTRL) );
	u32dat |= (u32)(1 << (port-1));
	Write_Reg(P0REG_WOLMODECTRL, u32dat & 0xFFFF);
	Write_Reg(P0REG_WOLMODECTRL+1, u32dat >> 16);

	ip1829drv_dbg("ip1829: -setWolStatusInSlaveMode...\n");
	return 0;
}

int getWolStatus(void *cdata, int len)
{
	int i;
	u32 u32dat;

	ip1829drv_dbg("ip1829: +getWolStatus...\n");
	if (sizeof(struct AllPortsSetting) != len)
		return -EINVAL;

	IP2Page(0);
	u32dat = (u32)( (Read_Reg(P0REG_WOLMODECTRL+1) << 16) | Read_Reg(P0REG_WOLMODECTRL) );
	for (i=0; i < (SWITCH_MAX_IPORT_CNT-1); i++)
	{
		((struct AllPortsSetting *)cdata) ->apdata[i] = (int)(u32dat & 0x1);
		u32dat = u32dat >> 1;
	}

	u32dat = (u32)( (Read_Reg(P0REG_WOLEVENT+1) << 16) | Read_Reg(P0REG_WOLEVENT) );
	ip1829drv_dbg("u32dat=0x%08x\n", u32dat);
	ip1829drv_dbg("cdata ->apdata=\n[");
	for (i=0; i < (SWITCH_MAX_IPORT_CNT-1); i++)
	{
		if (u32dat & 0x1)
			((struct AllPortsSetting *)cdata) ->apdata[i] = OP_WOL_STATUS_RDY4SLEEP;

		ip1829drv_dbg("%d ", ((struct AllPortsSetting *)cdata) ->apdata[i]);
		if (i%5 == 4)
			ip1829drv_dbg("   ");
		if (i%10 == 9)
			ip1829drv_dbg("\n ");
		u32dat = u32dat >> 1;
	}
	ip1829drv_dbg("]\n");
	ip1829drv_dbg("ip1829: -getWolStatus...\n");
	return 0;
}

int setCpuPortLink(void *cdata, int len)
{
	int clink;

	ip1829drv_dbg("ip1829: +setCpuPortLink...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	clink = ((struct GeneralSetting *)cdata) ->gdata;
	if (clink!=OP_CPU_PORT_NORMAL && clink!=OP_CPU_PORT_CPU)
		return -EINVAL;
	ip1829drv_dbg("clink=%d\n", clink);

	_WriteRegBits(0xC, PCREG_CPUMODE, 0, 1, clink);
	ip1829drv_dbg("ip1829: -setCpuPortLink...\n");
	return 0;
}

int getCpuPortLink(void *cdata, int len)
{
	ip1829drv_dbg("ip1829: +getCpuPortLink...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	((struct GeneralSetting *)cdata) ->gdata = (int)_ReadRegBits(0x0c, PCREG_CPUMODE, 0, 1);

	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	ip1829drv_dbg("ip1829: -getCpuPortLink...\n");
	return 0;
}

int setSTagFunc(void *cdata, int len)
{
	int en;

	ip1829drv_dbg("ip1829: +setSTagFunc...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	en = ((struct GeneralSetting *)cdata) ->gdata;
	if (en!=OP_FUNC_DISABLE && en!=OP_FUNC_ENABLE)
		return -EINVAL;
	ip1829drv_dbg("en=%d\n", en);

	_WriteRegBits(0xC, PCREG_CPUMODE, 1, 1, en);
	ip1829drv_dbg("ip1829: -setSTagFunc...\n");
	return 0;
}

int getSTagFunc(void *cdata, int len)
{
	ip1829drv_dbg("ip1829: +getSTagFunc...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	((struct GeneralSetting *)cdata) ->gdata = (int)_ReadRegBits(0xC, PCREG_CPUMODE, 1, 1);

	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	ip1829drv_dbg("ip1829: -getSTagFunc...\n");
	return 0;
}

int setSTagTypeLen(void *cdata, int len)
{
	u32 type, length;

	ip1829drv_dbg("ip1829: +setSTagTypeLen...\n");
	if (sizeof(struct STagTypeLenSetting) != len)
		return -EINVAL;

	length=((struct STagTypeLenSetting *)cdata) ->length;
	type = ((struct STagTypeLenSetting *)cdata) ->type;
	//	if (length > 0xFFFF)
	//		return -EINVAL;
	if (type > 0xFFFF)
		return -EINVAL;
	ip1829drv_dbg("length=%d\n", length);
	ip1829drv_dbg("type=0x%08x\n", type);

	IP2Page(0xC);
	Write_Reg(PCREG_SPTAG, (u16)type);

	ip1829drv_dbg("ip1829: -setSTagTypeLen...\n");
	return 0;
}

int getSTagTypeLen(void *cdata, int len)
{
	u16 u16dat;

	ip1829drv_dbg("ip1829: +getSTagTypeLen...\n");
	if (sizeof(struct STagTypeLenSetting) != len)
		return -EINVAL;

	IP2Page(0xC);
	u16dat= Read_Reg(PCREG_SPTAG);
	((struct STagTypeLenSetting *)cdata) ->length = (unsigned int)0;
	((struct STagTypeLenSetting *)cdata) ->type = (unsigned int)u16dat;
	ip1829drv_dbg("cdata ->length=%d\n", ((struct STagTypeLenSetting *)cdata) ->length);
	ip1829drv_dbg("cdata ->type=0x%08x\n", ((struct STagTypeLenSetting *)cdata) ->type);
	ip1829drv_dbg("ip1829: -getSTagTypeLen...\n");
	return 0;
}

//------------ PTP functions:common end  -----------------------
int setPtpEnable(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _setGeneralEnable(cdata, len, 0, P0REG_PTPCFG, 0);
	FUNC_MSG_OUT;
	return ret;
}

int getPtpEnable(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _getGeneralEnable(cdata, len, 0, P0REG_PTPCFG, 0);
	FUNC_MSG_OUT;
	return ret;
}

int setPtpToCpu(void *cdata, int len)
{
	int gdata;

	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	gdata = ((struct GeneralSetting *)cdata) ->gdata;
	if( gdata != OP_CAP_ACT_TO_CPU && gdata != OP_CAP_ACT_FORWARD )
	{
		ip1829drv_dbg("Error: gdata=%X\n", gdata);
		return -EINVAL;
	}

	ip1829drv_dbg("cdata ->gdata=%d\n", gdata);

	_WriteRegBits(0, P0REG_PTPCFG, 5, 1, gdata==OP_CAP_ACT_TO_CPU?0x0:0x1);

	FUNC_MSG_OUT;
	return 0;
}

int getPtpToCpu(void *cdata, int len)
{
	int gdata;

	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	gdata = (_ReadRegBits(0, P0REG_PTPCFG, 5, 1)==0x0?OP_CAP_ACT_TO_CPU:OP_CAP_ACT_FORWARD);
	((struct GeneralSetting *)cdata) ->gdata = gdata;

	ip1829drv_dbg("cdata ->gdata=%d\n", gdata);
	FUNC_MSG_OUT;
	return 0;
}


//------------ PTP functions:common end  -----------------------
int setJumboPktFunc(void *cdata, int len)
{
	unsigned long pm;
	int en;

	ip1829drv_dbg("ip1829: +setJumboPktFunc...\n");
	if (sizeof(struct PortmapSetting) != len)
		return -EINVAL;

	pm = ((struct PortmapSetting *)cdata) ->portmap;
	en = ((struct PortmapSetting *)cdata) ->pmdata;
	if (pm & 0xE0000000)
		return -EINVAL;
	if (en != OP_FUNC_ENABLE)
		return -EINVAL;
	ip1829drv_dbg("pm=0x%08x\n", (unsigned int)pm);
	ip1829drv_dbg("en=%d\n", en);

	IP2Page(0);
	Write_Reg(P0REG_JUMBO, (u16)(pm & 0xFFFF));
	Write_Reg(P0REG_JUMBO+1, (u16)(pm >> 16));

	ip1829drv_dbg("ip1829: -setJumboPktFunc...\n");
	return 0;
}

int getJumboPktFunc(void *cdata, int len)
{
	u32 u32dat;

	ip1829drv_dbg("ip1829: +getJumboPktFunc...\n");
	if (sizeof(struct PortmapSetting) != len)
		return -EINVAL;

	IP2Page(0);
	u32dat = (u32)( (Read_Reg(P0REG_JUMBO+1) << 16) | Read_Reg(P0REG_JUMBO) );
	((struct PortmapSetting *)cdata) ->portmap = (u32dat & 0x1FFFFFFF);
	((struct PortmapSetting *)cdata) ->pmdata = OP_FUNC_ENABLE;
	ip1829drv_dbg("cdata ->portmap=0x%08x\n", (unsigned int)((struct PortmapSetting *)cdata) ->portmap);
	ip1829drv_dbg("cdata ->pmdata=%d\n", ((struct PortmapSetting *)cdata) ->pmdata);
	ip1829drv_dbg("ip1829: -getJumboPktFunc...\n");
	return 0;
}

int set8021xFunc(void *cdata, int len)
{
	unsigned long pm;
	int en;

	ip1829drv_dbg("ip1829: +set8021xFunc...\n");
	if (sizeof(struct PortmapSetting) != len)
		return -EINVAL;

	pm = ((struct PortmapSetting *)cdata) ->portmap;
	en = ((struct PortmapSetting *)cdata) ->pmdata;
	if (pm & 0xF0000000)
		return -EINVAL;
	if (en != OP_FUNC_ENABLE)
		return -EINVAL;
	ip1829drv_dbg("pm=0x%08x\n", (unsigned int)pm);
	ip1829drv_dbg("en=%d\n", en);

	IP2Page(0);
	Write_Reg(P0REG_PORTLOCKEN, (u16)(pm & 0xFFFF));
	Write_Reg(P0REG_PORTLOCKEN+1, (u16)(pm >> 16));

	ip1829drv_dbg("ip1829: -set8021xFunc...\n");
	return 0;
}

int get8021xFunc(void *cdata, int len)
{
	u32 u32dat;

	ip1829drv_dbg("ip1829: +get8021xFunc...\n");
	if (sizeof(struct PortmapSetting) != len)
		return -EINVAL;

	IP2Page(0);
	u32dat = (u32)( (Read_Reg(P0REG_PORTLOCKEN+1) << 16) | Read_Reg(P0REG_PORTLOCKEN) );
	((struct PortmapSetting *)cdata) ->portmap = (u32dat & ALL_PHY_PORTS_LIST);
	((struct PortmapSetting *)cdata) ->pmdata = OP_FUNC_ENABLE;
	ip1829drv_dbg("cdata ->portmap=0x%08x\n", (unsigned int)((struct PortmapSetting *)cdata) ->portmap);
	ip1829drv_dbg("cdata ->pmdata=%d\n", ((struct PortmapSetting *)cdata) ->pmdata);
	ip1829drv_dbg("ip1829: -get8021xFunc...\n");
	return 0;
}

int setReg(void *cdata, int len)
{
	u8 page, reg;
	u16 val;

	ip1829drv_dbg("ip1829: +setReg...\n");
	if (sizeof(struct RegSetting) != len)
		return -EINVAL;

	page= ((struct RegSetting *)cdata) ->page;
	reg = ((struct RegSetting *)cdata) ->reg;
	val = ((struct RegSetting *)cdata) ->val;
	if (page > 0xC)
		return -EINVAL;
	ip1829drv_dbg("page=0x%x\n", page);
	ip1829drv_dbg("reg=0x%02X\n", reg);
	ip1829drv_dbg("val=0x%04x\n", val);

	IP2Page(page);
	//	ip1829drv_dbg("RegList[%x][0x%02X]=0x%04x\n", pg, P2REG_VLANCFG,
	//				RegList[pg][P2REG_VLANCFG]);
	Write_Reg(reg, val);

	//	ip1829drv_dbg("RegList[%x][0x%02X]=0x%04x\n", pg, P2REG_VLANCFG,
	//				RegList[pg][P2REG_VLANCFG]);
	ip1829drv_dbg("ip1829: -setReg...\n");
	return 0;
}

int getReg(void *cdata, int len)
{
	u8 page, reg;
	u16 u16dat;

	ip1829drv_dbg("ip1829: +getReg...\n");
	if (sizeof(struct RegSetting) != len)
		return -EINVAL;

	page= ((struct RegSetting *)cdata) ->page;
	reg = ((struct RegSetting *)cdata) ->reg;
	if (page > 0xC)
		return -EINVAL;
	ip1829drv_dbg("page=0x%x\n", page);
	ip1829drv_dbg("reg=0x%02X\n", reg);

	IP2Page(page);
	//	ip1829drv_dbg("RegList[%x][0x%02X]=0x%04x\n", pg, P2REG_VLANCFG,
	//				RegList[pg][P2REG_VLANCFG]);
	u16dat = Read_Reg(reg);
	ip1829drv_dbg("u16dat=0x%04x\n", u16dat);
	((struct RegSetting *)cdata) ->val = (unsigned short)u16dat;

	//	ip1829drv_dbg("RegList[%x][0x%02X]=0x%04x\n", pg, P2REG_VLANCFG,
	//				RegList[pg][P2REG_VLANCFG]);
	ip1829drv_dbg("cdata ->val=0x%04x\n", ((struct RegSetting *)cdata) ->val);
	ip1829drv_dbg("ip1829: -getReg...\n");
	return 0;
}

int setCPUReg(void *cdata, int len)
{
	unsigned long reg;
	int val;
	volatile unsigned int *p;

	FUNC_MSG_IN;

	if (sizeof(struct PortmapSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	reg = ((struct PortmapSetting *)cdata) ->portmap;
	val = ((struct PortmapSetting *)cdata) ->pmdata;
	p = (unsigned int *)(reg);
	*p = val;
	ip1829delay();

	ip1829drv_dbg("reg=%lX\n", reg);
	ip1829drv_dbg("set val=%X\n", val);

	FUNC_MSG_OUT;
	return 0;
}

int getCPUReg(void *cdata, int len)
{
	unsigned long reg;
	int val;
	volatile unsigned int *p;

	FUNC_MSG_IN;

	if (sizeof(struct PortmapSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	reg = ((struct PortmapSetting *)cdata) ->portmap;
	p = (unsigned int *)(reg);
	val = *p;
	((struct PortmapSetting *)cdata) ->pmdata = val;
	ip1829delay();

	ip1829drv_dbg("reg=%lX\n", reg);
	ip1829drv_dbg("get val=%X\n", val);

	FUNC_MSG_OUT;
	return 0;
}

int setSwitchRestart(void *cdata, int len)
{
	int en;
	FUNC_MSG_IN;

	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	en = ((struct GeneralSetting *)cdata) ->gdata;
	if(en!=OP_FUNC_ENABLE)
	{
		ip1829drv_dbg("Error: en=%d\n", en);
		return -EINVAL;
	}
	ip1829drv_dbg("reset the switch of IP1829, all parameters are maintained\n");
	_WriteRegBits(0xC, PCREG_SW_RESET, 0, 1, en);

	FUNC_MSG_OUT;
	return 0;
}

int setSwitchReset(void *cdata, int len)
{
	int en;
	FUNC_MSG_IN;

	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	en = ((struct GeneralSetting *)cdata) ->gdata;
	if(en!=OP_FUNC_ENABLE)
	{
		ip1829drv_dbg("Error: en=%d\n", en);
		return -EINVAL;
	}
	ip1829drv_dbg("reset the switch of IP1829, all parameters are reset to default\n");
	_WriteRegBits(0xC, PCREG_SW_RESET, 4, 1, en);

	FUNC_MSG_OUT;
	return 0;
}

int setCpuIfSpeed(void *cdata, int len)
{
	FUNC_MSG_IN;

	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	CPU_IF_SPEED_NORMAL = ((struct GeneralSetting *)cdata) ->gdata;
	ic_mdio_set_divisor(CPU_IF_SPEED_NORMAL?3:0);
	ip1829drv_dbg("ip1829: set CPU I/F speed to %s\n",CPU_IF_SPEED_NORMAL?"Normal":"High");

	FUNC_MSG_OUT;
	return 0;
}

int setMstpFunc(void *cdata, int len)
{
	int en;

	ip1829drv_dbg("ip1829: +setMstpFunc...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	en = ((struct GeneralSetting *)cdata) ->gdata;
	if (en!=OP_FUNC_DISABLE && en!=OP_FUNC_ENABLE)
		return -EINVAL;
	ip1829drv_dbg("en=%d\n", en);

	_WriteRegBits(2, P2REG_VLANCFG, 6, 1, en);
	ip1829drv_dbg("ip1829: -setMstpFunc...\n");
	return 0;
}

int getMstpFunc(void *cdata, int len)
{
	ip1829drv_dbg("ip1829: +getMstpFunc...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	((struct GeneralSetting *)cdata) ->gdata = (int)_ReadRegBits(2, P2REG_VLANCFG, 6, 1);
	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	ip1829drv_dbg("ip1829: -getMstpFunc...\n");
	return 0;
}

int setTrunkHashMthd(void *cdata, int len)
{
	int mthd;

	ip1829drv_dbg("ip1829: +setTrunkHashMthd...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	mthd = ((struct GeneralSetting *)cdata) ->gdata;
	if (mthd < OP_TRUNK_HASH_METHOD_PORT_ID || mthd > OP_TRUNK_HASH_METHOD_SP)
		return -EINVAL;
	ip1829drv_dbg("mthd=%d\n", mthd);

	_WriteRegBits(1, P1REG_TRUNKCFG, 0, 3, mthd);
	ip1829drv_dbg("ip1829: -setTrunkHashMthd...\n");
	return 0;
}

int getTrunkHashMthd(void *cdata, int len)
{
	int mthd;

	ip1829drv_dbg("ip1829: +getTrunkHashMthd...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	mthd = (int)_ReadRegBits(1, P1REG_TRUNKCFG, 0, 3);
	((struct GeneralSetting *)cdata) ->gdata = mthd;
	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	ip1829drv_dbg("ip1829: -getTrunkHashMthd...\n");
	return 0;
}

int setTrunkMbr(void *cdata, int len)
{
	u32 pmask, state, u32dat;

	ip1829drv_dbg("ip1829: +setTrunkMbr...\n");
	if (sizeof(struct TrunkMemberSetting) != len)
		return -EINVAL;

	pmask = ((struct TrunkMemberSetting *)cdata) ->portmask;
	state = ((struct TrunkMemberSetting *)cdata) ->tstate;
	if ((pmask & 0xF0FF0000) || !pmask)
		return -EINVAL;
	if ((state & 0xF0FF0000) || (state & ~pmask))
		return -EINVAL;
	ip1829drv_dbg("pmask=0x%08X\n", pmask);
	ip1829drv_dbg("state=0x%08X\n", state);

	IP2Page(1);
	u32dat= (u32)( Read_Reg(P1REG_TRUNKGRP) << 16 | Read_Reg(P1REG_TRUNKCFG) );
	pmask = (u32)( (pmask&0x0F000000) | ((pmask&0xFFFF) << 8) );
	state = (u32)( (state&0x0F000000) | ((state&0xFFFF) << 8) );
	u32dat &= (~pmask);
	u32dat |= state;
	Write_Reg(P1REG_TRUNKGRP, (u32dat >> 16));
	Write_Reg(P1REG_TRUNKCFG, (u32dat & 0xFFFF));

	ip1829drv_dbg("ip1829: -setTrunkMbr...\n");
	return 0;
}

int getTrunkMbr(void *cdata, int len)
{
	u32 pmask, u32dat;

	ip1829drv_dbg("ip1829: +getTrunkMbr...\n");
	if (sizeof(struct TrunkMemberSetting) != len)
		return -EINVAL;

	pmask = ((struct TrunkMemberSetting *)cdata) ->portmask;
	if ((pmask & 0xF0FF0000) || !pmask)
		return -EINVAL;
	ip1829drv_dbg("pmask=0x%08X\n", pmask);

	IP2Page(1);
	u32dat = (u32)( Read_Reg(P1REG_TRUNKGRP) << 16 | Read_Reg(P1REG_TRUNKCFG) );
	u32dat = (u32dat&0x0F000000) | ((u32dat&0x00FFFF00) >> 8);
	((struct TrunkMemberSetting *)cdata) ->tstate = u32dat & pmask;
	ip1829drv_dbg("cdata ->tstate=0x%08x\n", (unsigned int)((struct TrunkMemberSetting *)cdata) ->tstate);
	ip1829drv_dbg("ip1829: -getTrunkMbr...\n");
	return 0;
}

int setCpuNCareTrunkAndVlan(void *cdata, int len)
{
	int en;

	ip1829drv_dbg("ip1829: +setCpuNCareTrunkAndVlan...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	en = ((struct GeneralSetting *)cdata) ->gdata;
	if (en!=OP_FUNC_DISABLE && en!=OP_FUNC_ENABLE)
		return -EINVAL;
	ip1829drv_dbg("en=%d\n", en);

	_WriteRegBits(1, P1REG_TRUNKCFG, 4, 1, en);
	ip1829drv_dbg("ip1829: -setCpuNCareTrunkAndVlan...\n");
	return 0;
}

int getCpuNCareTrunkAndVlan(void *cdata, int len)
{
	int en;

	ip1829drv_dbg("ip1829: +getCpuNCareTrunkAndVlan...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;
	en = (int)_ReadRegBits(1, P1REG_TRUNKCFG, 4, 1);
	((struct GeneralSetting *)cdata) ->gdata = en;
	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	ip1829drv_dbg("ip1829: -getCpuNCareTrunkAndVlan...\n");
	return 0;
}

int setS1PktModify(void *cdata, int len)
{
	int modify;

	ip1829drv_dbg("ip1829: +setS1PktModify...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	modify = ((struct GeneralSetting *)cdata) ->gdata;
	if (modify!=OP_SNIFFER1_PKT_MODIFY && modify!=OP_SNIFFER1_PKT_KEEP)
		return -EINVAL;
	ip1829drv_dbg("modify=%d\n", modify);

	_WriteRegBits(1, P1REG_SNIFDEST1, 15, 1, modify);
	ip1829drv_dbg("ip1829: -setS1PktModify...\n");
	return 0;
}

int getS1PktModify(void *cdata, int len)
{
	ip1829drv_dbg("ip1829: +getS1PktModify...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	((struct GeneralSetting *)cdata) ->gdata = (int)_ReadRegBits(1, P1REG_SNIFDEST1, 15, 1);

	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	ip1829drv_dbg("ip1829: -getS1PktModify...\n");
	return 0;
}

int setS1TM4CpuSTag(void *cdata, int len)
{
	int modify;

	ip1829drv_dbg("ip1829: +setS1TM4CpuSTag...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	modify = ((struct GeneralSetting *)cdata) ->gdata;
	if (modify!=OP_SNIFFER1_TAG_KEEP && modify!=OP_SNIFFER1_TAG_MODIFY)
		return -EINVAL;
	ip1829drv_dbg("modify=%d\n", modify);

	_WriteRegBits(1, P1REG_MISCCFG, 5, 1, modify);
	ip1829drv_dbg("ip1829: -setS1TM4CpuSTag...\n");
	return 0;
}

int getS1TM4CpuSTag(void *cdata, int len)
{
	ip1829drv_dbg("ip1829: +getS1TM4CpuSTag...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	((struct GeneralSetting *)cdata) ->gdata = (int)_ReadRegBits(1, P1REG_MISCCFG, 5, 1);

	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	ip1829drv_dbg("ip1829: -getS1TM4CpuSTag...\n");
	return 0;
}

int setS1TM4Acl2Cpu(void *cdata, int len)
{
	int modify;

	ip1829drv_dbg("ip1829: +setS1TM4Acl2Cpu...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	modify = ((struct GeneralSetting *)cdata) ->gdata;
	if (modify!=OP_SNIFFER1_TAG_KEEP && modify!=OP_SNIFFER1_TAG_MODIFY)
		return -EINVAL;
	ip1829drv_dbg("modify=%d\n", modify);

	_WriteRegBits(1, P1REG_MISCCFG, 6, 1, modify);
	ip1829drv_dbg("ip1829: -setS1TM4Acl2Cpu...\n");
	return 0;
}

int getS1TM4Acl2Cpu(void *cdata, int len)
{
	ip1829drv_dbg("ip1829: +getS1TM4Acl2Cpu...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	((struct GeneralSetting *)cdata) ->gdata = (int)_ReadRegBits(1, P1REG_MISCCFG, 6, 1);

	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	ip1829drv_dbg("ip1829: -getS1TM4Acl2Cpu...\n");
	return 0;
}

int setS1TM4Pkt2MPort(void *cdata, int len)
{
	int modify;

	ip1829drv_dbg("ip1829: +setS1TM4Pkt2MPort...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	modify = ((struct GeneralSetting *)cdata) ->gdata;
	if (modify!=OP_SNIFFER1_TAG_KEEP && modify!=OP_SNIFFER1_TAG_MODIFY)
		return -EINVAL;
	ip1829drv_dbg("modify=%d\n", modify);

	_WriteRegBits(1, P1REG_MISCCFG, 7, 1, modify);
	ip1829drv_dbg("ip1829: -setS1TM4Pkt2MPort...\n");
	return 0;
}

int getS1TM4Pkt2MPort(void *cdata, int len)
{
	ip1829drv_dbg("ip1829: +getS1TM4Pkt2MPort...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	((struct GeneralSetting *)cdata) ->gdata = (int)_ReadRegBits(1, P1REG_MISCCFG, 7, 1);

	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	ip1829drv_dbg("ip1829: -getS1TM4Pkt2MPort...\n");
	return 0;
}

int setS2LTT4Grp1(void *cdata, int len)
{
	int ltt;

	ip1829drv_dbg("ip1829: +setS2LTT4Grp1...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	ltt = ((struct GeneralSetting *)cdata) ->gdata;
	if (ltt!=OP_SNIFFER2_LUT_TRIGGER_TARGET_DA && ltt!=OP_SNIFFER2_LUT_TRIGGER_TARGET_SA)
		return -EINVAL;
	ip1829drv_dbg("ltt=%d\n", ltt);

	_WriteRegBits(1, P1REG_SNIF2DEST1, 13, 1, ltt);
	ip1829drv_dbg("ip1829: -setS2LTT4Grp1...\n");
	return 0;
}

int getS2LTT4Grp1(void *cdata, int len)
{
	ip1829drv_dbg("ip1829: +getS2LTT4Grp1...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	((struct GeneralSetting *)cdata) ->gdata = (int)_ReadRegBits(1, P1REG_SNIF2DEST1, 13, 1);

	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	ip1829drv_dbg("ip1829: -getS2LTT4Grp1...\n");
	return 0;
}

int setS2LTT4Grp2(void *cdata, int len)
{
	int ltt;

	ip1829drv_dbg("ip1829: +setS2LTT4Grp2...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	ltt = ((struct GeneralSetting *)cdata) ->gdata;
	if (ltt!=OP_SNIFFER2_LUT_TRIGGER_TARGET_DA && ltt!=OP_SNIFFER2_LUT_TRIGGER_TARGET_SA)
		return -EINVAL;
	ip1829drv_dbg("ltt=%d\n", ltt);

	_WriteRegBits(1, P1REG_SNIF2DEST1, 14, 1, ltt);
	ip1829drv_dbg("ip1829: -setS2LTT4Grp2...\n");
	return 0;
}

int getS2LTT4Grp2(void *cdata, int len)
{
	ip1829drv_dbg("ip1829: +getS2LTT4Grp2...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	((struct GeneralSetting *)cdata) ->gdata = (int)_ReadRegBits(1, P1REG_SNIF2DEST1, 14, 1);

	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	ip1829drv_dbg("ip1829: -getS2LTT4Grp2...\n");
	return 0;
}

int setMStormNBlockIpPkt(void *cdata, int len)
{
	int nb;

	ip1829drv_dbg("ip1829: +setMStormNBlockIpPkt...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	nb = ((struct GeneralSetting *)cdata) ->gdata;
	if (nb!=OP_FUNC_DISABLE && nb!=OP_FUNC_ENABLE)
		return -EINVAL;
	ip1829drv_dbg("nb=%d\n", nb);

	_WriteRegBits(1, P1REG_BSTORMTHRESH, 11, 1, nb);
	ip1829drv_dbg("ip1829: -setMStormNBlockIpPkt...\n");
	return 0;
}

int getMStormNBlockIpPkt(void *cdata, int len)
{
	ip1829drv_dbg("ip1829: +getMStormNBlockIpPkt...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	((struct GeneralSetting *)cdata) ->gdata = (int)_ReadRegBits(1, P1REG_BSTORMTHRESH, 11, 1);

	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	ip1829drv_dbg("ip1829: -getMStormNBlockIpPkt...\n");
	return 0;
}

int setMStormIgnr01005EXXXXXX(void *cdata, int len)
{
	int ignr;

	ip1829drv_dbg("ip1829: +setMStormIgnr01005EXXXXXX...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	ignr = ((struct GeneralSetting *)cdata) ->gdata;
	if (ignr!=OP_FUNC_DISABLE && ignr!=OP_FUNC_ENABLE)
		return -EINVAL;
	ip1829drv_dbg("ignr=%d\n", ignr);

	_WriteRegBits(1, P1REG_BSTORMTHRESH, 12, 1, ignr);
	ip1829drv_dbg("ip1829: -setMStormIgnr01005EXXXXXX...\n");
	return 0;
}

int getMStormIgnr01005EXXXXXX(void *cdata, int len)
{
	ip1829drv_dbg("ip1829: +getMStormIgnr01005EXXXXXX...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	((struct GeneralSetting *)cdata) ->gdata = (int)_ReadRegBits(1, P1REG_BSTORMTHRESH, 12, 1);

	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	ip1829drv_dbg("ip1829: -getMStormIgnr01005EXXXXXX...\n");
	return 0;
}

int setEocBlockClr(void *cdata, int len)
{
	int en;

	ip1829drv_dbg("ip1829: +setEocBlockClr...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	en = ((struct GeneralSetting *)cdata) ->gdata;
	if (en != OP_FUNC_ENABLE)
		return -EINVAL;
	ip1829drv_dbg("en=%d\n", en);

	_WriteRegBits(0, P0REG_EOCCFG1, 15, 1, 0x01);
	ip1829drv_dbg("ip1829: -setEocBlockClr...\n");
	return 0;
}

int setEocClrBlockWhenRcvGood(void *cdata, int len)
{
	int en;

	ip1829drv_dbg("ip1829: +setEocClrBlockWhenRcvGood...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	en = ((struct GeneralSetting *)cdata) ->gdata;
	if (en!=OP_FUNC_DISABLE && en!=OP_FUNC_ENABLE)
		return -EINVAL;
	ip1829drv_dbg("en=%d\n", en);

	_WriteRegBits(0, P0REG_EOCCFG1, 13, 1, en);
	ip1829drv_dbg("ip1829: -setEocClrBlockWhenRcvGood...\n");
	return 0;
}

int getEocClrBlockWhenRcvGood(void *cdata, int len)
{
	ip1829drv_dbg("ip1829: +getEocClrBlockWhenRcvGood...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	((struct GeneralSetting *)cdata) ->gdata = (int)_ReadRegBits(0, P0REG_EOCCFG1, 13, 1);

	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	ip1829drv_dbg("ip1829: -getEocClrBlockWhenRcvGood...\n");
	return 0;
}

int setLdSMACB40(void *cdata, int len)
{
	int b40;

	ip1829drv_dbg("ip1829: +setLdSMACB40...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	b40 = ((struct GeneralSetting *)cdata) ->gdata;
	if (b40!=0 && b40!=1)
		return -EINVAL;
	ip1829drv_dbg("b40=%d\n", b40);

	_WriteRegBits(0, P0REG_LDCONFIG1, 14, 1, b40);
	ip1829drv_dbg("ip1829: -setLdSMACB40...\n");
	return 0;
}

int getLdSMACB40(void *cdata, int len)
{
	ip1829drv_dbg("ip1829: +getLdSMACB40...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	((struct GeneralSetting *)cdata) ->gdata = (int)_ReadRegBits(0, P0REG_LDCONFIG1, 14, 1);

	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	ip1829drv_dbg("ip1829: -getLdSMACB40...\n");
	return 0;
}

int setLdRerandom(void *cdata, int len)
{
	int re;

	ip1829drv_dbg("ip1829: +setLdRerandom...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	re = ((struct GeneralSetting *)cdata) ->gdata;
	if (re!=OP_FUNC_DISABLE && re!=OP_FUNC_ENABLE)
		return -EINVAL;
	ip1829drv_dbg("re=%d\n", re);

	_WriteRegBits(0, P0REG_LDCONFIG1, 15, 1, re);
	ip1829drv_dbg("ip1829: -setLdRerandom...\n");
	return 0;
}

int getLdRerandom(void *cdata, int len)
{
	ip1829drv_dbg("ip1829: +getLdRerandom...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	((struct GeneralSetting *)cdata) ->gdata = (int)_ReadRegBits(0, P0REG_LDCONFIG1, 15, 1);

	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	ip1829drv_dbg("ip1829: -getLdRerandom...\n");
	return 0;
}

int setLdDMAC(void *cdata, int len)
{
	int i;
	u8  da[6];
	u16 u16dat;

	ip1829drv_dbg("ip1829: +setLdDMAC...\n");
	if (sizeof(struct LDDASetting) != len)
		return -EINVAL;

	ip1829drv_dbg("da=[");
	for (i=0; i < 6; i++)
	{
		da[i] = ((struct LDDASetting *)cdata) ->da[i];
		ip1829drv_dbg("0x%02X, ", da[i]);
	}
	ip1829drv_dbg("]\n");

	IP2Page(0);
	for (i=0; i < 3; i++)
	{
		u16dat = (u16)( (da[i*2] << 8) | da[i*2+1] );
		Write_Reg(P0REG_LDDA0+2-i, u16dat);
	}

	ip1829drv_dbg("ip1829: -setLdDMAC...\n");
	return 0;
}

int getLdDMAC(void *cdata, int len)
{
	int i;
	u16 u16dat;

	ip1829drv_dbg("ip1829: +getLdDMAC...\n");
	if (sizeof(struct LDDASetting) != len)
		return -EINVAL;

	IP2Page(0);
	for (i=0; i < 3; i++)
	{
		u16dat = Read_Reg(P0REG_LDDA0+2-i);
		((struct LDDASetting *)cdata) ->da[i*2] = (u8)(u16dat >> 8);
		((struct LDDASetting *)cdata) ->da[i*2+1] = (u8)(u16dat & 0xFF);
	}
	ip1829drv_dbg("cdata ->da[]=[");
	for (i=0; i < 6; i++)
		ip1829drv_dbg("0x%02X, ", ((struct LDDASetting *)cdata) ->da[i]);
	ip1829drv_dbg("]\n");
	ip1829drv_dbg("ip1829: -getLdDMAC...\n");
	return 0;
}

int setLdEtherType(void *cdata, int len)
{
	int etype;

	ip1829drv_dbg("ip1829: +setLdEtherType...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	etype = ((struct GeneralSetting *)cdata) ->gdata;
	if (etype<0 && etype>0xFFFF)
		return -EINVAL;
	ip1829drv_dbg("etype=%d\n", etype);

	IP2Page(0);
	Write_Reg(P0REG_LDETYPE, (u16)etype);

	ip1829drv_dbg("ip1829: -setLdEtherType...\n");
	return 0;
}

int getLdEtherType(void *cdata, int len)
{
	ip1829drv_dbg("ip1829: +getLdEtherType...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	IP2Page(0);
	((struct GeneralSetting *)cdata) ->gdata = (int)(Read_Reg(P0REG_LDETYPE));
	ip1829drv_dbg("cdata ->gdata=0x%04x\n", ((struct GeneralSetting *)cdata) ->gdata);
	ip1829drv_dbg("ip1829: -getLdEtherType...\n");
	return 0;
}

int setLdSubType(void *cdata, int len)
{
	int stype;

	ip1829drv_dbg("ip1829: +setLdSubType...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	stype = ((struct GeneralSetting *)cdata) ->gdata;
	if (stype<0 && stype>0xFFFF)
		return -EINVAL;
	ip1829drv_dbg("stype=%d\n", stype);

	IP2Page(0);
	Write_Reg(P0REG_LDSUBTYPE, (u16)stype);

	ip1829drv_dbg("ip1829: -setLdSubType...\n");
	return 0;
}

int getLdSubType(void *cdata, int len)
{
	ip1829drv_dbg("ip1829: +getLdSubType...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	IP2Page(0);
	((struct GeneralSetting *)cdata) ->gdata = (int)(Read_Reg(P0REG_LDSUBTYPE));
	ip1829drv_dbg("cdata ->gdata=0x%04x\n", ((struct GeneralSetting *)cdata) ->gdata);
	ip1829drv_dbg("ip1829: -getLdSubType...\n");
	return 0;
}

int setLdDeviceID(void *cdata, int len)
{
	int devid;

	ip1829drv_dbg("ip1829: +setLdDeviceID...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	devid = ((struct GeneralSetting *)cdata) ->gdata;
	if (devid<0 && devid>0xFF)
		return -EINVAL;
	ip1829drv_dbg("devid=%d\n", devid);

	IP2Page(0);
	Write_Reg(P0REG_LDDEVID, (u16)devid);

	ip1829drv_dbg("ip1829: -setLdDeviceID...\n");
	return 0;
}

int getLdDeviceID(void *cdata, int len)
{
	ip1829drv_dbg("ip1829: +getLdDeviceID...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	IP2Page(0);
	((struct GeneralSetting *)cdata) ->gdata = (int)(Read_Reg(P0REG_LDDEVID));
	ip1829drv_dbg("cdata ->gdata=0x%04x\n", ((struct GeneralSetting *)cdata) ->gdata);
	ip1829drv_dbg("ip1829: -getLdDeviceID...\n");
	return 0;
}

int setWolWakeIfTxGetAnyPkt(void *cdata, int len)
{
	int en;

	ip1829drv_dbg("ip1829: +setWolWakeIfTxGetAnyPkt...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	en = ((struct GeneralSetting *)cdata) ->gdata;
	if (en!=OP_FUNC_DISABLE && en!=OP_FUNC_ENABLE)
		return -EINVAL;
	ip1829drv_dbg("en=%d\n", en);

	_WriteRegBits(0, P0REG_WOLCFG, 7, 1, (en == OP_FUNC_ENABLE)?0:1);
	ip1829drv_dbg("ip1829: -setWolWakeIfTxGetAnyPkt...\n");
	return 0;
}

int getWolWakeIfTxGetAnyPkt(void *cdata, int len)
{
	ip1829drv_dbg("ip1829: +getWolWakeIfTxGetAnyPkt...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	((struct GeneralSetting *)cdata) ->gdata = (int)~(_ReadRegBits(0, P0REG_WOLCFG, 7, 1));

	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	ip1829drv_dbg("ip1829: -getWolWakeIfTxGetAnyPkt...\n");
	return 0;
}

int setWolWakeIfRxGetAnyPkt(void *cdata, int len)
{
	int en;

	ip1829drv_dbg("ip1829: +setWolWakeIfRxGetAnyPkt...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	en = ((struct GeneralSetting *)cdata) ->gdata;
	if (en!=OP_FUNC_DISABLE && en!=OP_FUNC_ENABLE)
		return -EINVAL;
	ip1829drv_dbg("en=%d\n", en);

	_WriteRegBits(0, P0REG_WOLCFG, 8, 1, (en == OP_FUNC_ENABLE)?0:1);
	ip1829drv_dbg("ip1829: -setWolWakeIfRxGetAnyPkt...\n");
	return 0;
}

int getWolWakeIfRxGetAnyPkt(void *cdata, int len)
{
	ip1829drv_dbg("ip1829: +getWolWakeIfRxGetAnyPkt...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	((struct GeneralSetting *)cdata) ->gdata = (int)~(_ReadRegBits(0, P0REG_WOLCFG, 8, 1));

	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	ip1829drv_dbg("ip1829: -getWolWakeIfRxGetAnyPkt...\n");
	return 0;
}

int setWolWakeIfMatchAcl2Cpu(void *cdata, int len)
{
	int en;

	ip1829drv_dbg("ip1829: +setWolWakeIfMatchAcl2Cpu...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	en = ((struct GeneralSetting *)cdata) ->gdata;
	if (en!=OP_FUNC_DISABLE && en!=OP_FUNC_ENABLE)
		return -EINVAL;
	ip1829drv_dbg("en=%d\n", en);

	_WriteRegBits(0, P0REG_WOLCFG, 10, 1, en);
	ip1829drv_dbg("ip1829: -setWolWakeIfMatchAcl2Cpu...\n");
	return 0;
}

int getWolWakeIfMatchAcl2Cpu(void *cdata, int len)
{
	ip1829drv_dbg("ip1829: +getWolWakeIfMatchAcl2Cpu...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	((struct GeneralSetting *)cdata) ->gdata = (int)(_ReadRegBits(0, P0REG_WOLCFG, 10, 1));

	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	ip1829drv_dbg("ip1829: -getWolWakeIfMatchAcl2Cpu...\n");
	return 0;
}

int setMACLoopBackFunc(void *cdata, int len)
{
	unsigned long pm;
	int en;

	ip1829drv_dbg("ip1829: +setMACLoopBackFunc...\n");
	if (sizeof(struct PortmapSetting) != len)
		return -EINVAL;

	pm = ((struct PortmapSetting *)cdata) ->portmap;
	en = ((struct PortmapSetting *)cdata) ->pmdata;
	if (pm & 0xE0000000)
		return -EINVAL;
	if (en != OP_FUNC_ENABLE)
		return -EINVAL;
	ip1829drv_dbg("pm=0x%08x\n", (unsigned int)pm);
	ip1829drv_dbg("en=%d\n", en);

	IP2Page(0);
	Write_Reg(P0REG_MACLOOPB, (u16)(pm & 0xFFFF));
	Write_Reg(P0REG_MACLOOPB+1, (u16)(pm >> 16));

	ip1829drv_dbg("ip1829: -setMACLoopBackFunc...\n");
	return 0;
}

int getMACLoopBackFunc(void *cdata, int len)
{
	u32 u32dat;

	ip1829drv_dbg("ip1829: +getMACLoopBackFunc...\n");
	if (sizeof(struct PortmapSetting) != len)
		return -EINVAL;

	IP2Page(0);
	u32dat = (u32)( (Read_Reg(P0REG_MACLOOPB+1) << 16) | Read_Reg(P0REG_MACLOOPB) );
	((struct PortmapSetting *)cdata) ->portmap = (u32dat & 0x1FFFFFFF);
	((struct PortmapSetting *)cdata) ->pmdata = OP_FUNC_ENABLE;
	ip1829drv_dbg("cdata ->portmap=0x%08x\n", (unsigned int)((struct PortmapSetting *)cdata) ->portmap);
	ip1829drv_dbg("cdata ->pmdata=%d\n", ((struct PortmapSetting *)cdata) ->pmdata);
	ip1829drv_dbg("ip1829: -getMACLoopBackFunc...\n");
	return 0;
}

int setPausePktFunc(void *cdata, int len)
{
	int en;

	ip1829drv_dbg("ip1829: +setPausePktFunc...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	en = ((struct GeneralSetting *)cdata) ->gdata;
	if (en!=OP_FUNC_DISABLE && en!=OP_FUNC_ENABLE)
		return -EINVAL;
	ip1829drv_dbg("en=%d\n", en);

	_WriteRegBits(1, P1REG_FWDPAUSEEN1, 13, 1, en);
	ip1829drv_dbg("ip1829: -setPausePktFunc...\n");
	return 0;
}

int getPausePktFunc(void *cdata, int len)
{
	ip1829drv_dbg("ip1829: +getPausePktFunc...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	((struct GeneralSetting *)cdata) ->gdata = (int)_ReadRegBits(1, P1REG_FWDPAUSEEN1, 13, 1);

	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	ip1829drv_dbg("ip1829: -getPausePktFunc...\n");
	return 0;
}

int setPausePktDest(void *cdata, int len)
{
	unsigned long pm;
	int en;
	u32 u32dat;

	ip1829drv_dbg("ip1829: +setPausePktDest...\n");
	if (sizeof(struct PortmapSetting) != len)
		return -EINVAL;

	pm = ((struct PortmapSetting *)cdata) ->portmap;
	en = ((struct PortmapSetting *)cdata) ->pmdata;
	if (pm & 0xE0000000)
		return -EINVAL;
	if (en != OP_FUNC_ENABLE)
		return -EINVAL;
	ip1829drv_dbg("pm=0x%08x\n", (unsigned int)pm);
	ip1829drv_dbg("en=%d\n", en);

	IP2Page(1);
	u32dat = (u32)( (Read_Reg(P1REG_FWDPAUSEEN1) << 16) | Read_Reg(P1REG_FWDPAUSEEN) );
	u32dat &= 0xE0000000;
	u32dat |= pm;
	Write_Reg(P1REG_FWDPAUSEEN, (u16)(u32dat & 0xFFFF));
	Write_Reg(P1REG_FWDPAUSEEN1, (u16)(u32dat >> 16));

	ip1829drv_dbg("ip1829: -setPausePktDest...\n");
	return 0;
}

int getPausePktDest(void *cdata, int len)
{
	u32 u32dat;

	ip1829drv_dbg("ip1829: +getPausePktDest...\n");
	if (sizeof(struct PortmapSetting) != len)
		return -EINVAL;

	IP2Page(1);
	u32dat = (u32)( (Read_Reg(P1REG_FWDPAUSEEN1) << 16) | Read_Reg(P1REG_FWDPAUSEEN) );
	((struct PortmapSetting *)cdata) ->portmap = (u32dat & 0x1FFFFFFF);
	((struct PortmapSetting *)cdata) ->pmdata = OP_FUNC_ENABLE;
	ip1829drv_dbg("cdata ->portmap=0x%08x\n", (unsigned int)((struct PortmapSetting *)cdata) ->portmap);
	ip1829drv_dbg("cdata ->pmdata=%d\n", ((struct PortmapSetting *)cdata) ->pmdata);
	ip1829drv_dbg("ip1829: -getPausePktDest...\n");
	return 0;
}

int setLocalTrafficFunc(void *cdata, int len)
{
	unsigned long pm;
	int en;

	ip1829drv_dbg("ip1829: +setLocalTrafficFunc...\n");
	if (sizeof(struct PortmapSetting) != len)
		return -EINVAL;

	pm = ((struct PortmapSetting *)cdata) ->portmap;
	en = ((struct PortmapSetting *)cdata) ->pmdata;
	if (pm & 0xE0000000)
		return -EINVAL;
	if (en != OP_FUNC_ENABLE)
		return -EINVAL;
	ip1829drv_dbg("pm=0x%08x\n", (unsigned int)pm);
	ip1829drv_dbg("en=%d\n", en);

	IP2Page(2);
	Write_Reg(P2REG_VLANLOCAL, (u16)(pm & 0xFFFF));
	Write_Reg(P2REG_VLANLOCAL+1, (u16)(pm >> 16));

	ip1829drv_dbg("ip1829: -setLocalTrafficFunc...\n");
	return 0;
}

int getLocalTrafficFunc(void *cdata, int len)
{
	u32 u32dat;

	ip1829drv_dbg("ip1829: +getLocalTrafficFunc...\n");
	if (sizeof(struct PortmapSetting) != len)
		return -EINVAL;

	IP2Page(2);
	u32dat = (u32)( (Read_Reg(P2REG_VLANLOCAL+1) << 16) | Read_Reg(P2REG_VLANLOCAL) );
	((struct PortmapSetting *)cdata) ->portmap = (u32dat & 0x1FFFFFFF);
	((struct PortmapSetting *)cdata) ->pmdata = OP_FUNC_ENABLE;
	ip1829drv_dbg("cdata ->portmap=0x%08x\n", (unsigned int)((struct PortmapSetting *)cdata) ->portmap);
	ip1829drv_dbg("cdata ->pmdata=%d\n", ((struct PortmapSetting *)cdata) ->pmdata);
	ip1829drv_dbg("ip1829: -getLocalTrafficFunc...\n");
	return 0;
}

int setMACReset(void *cdata, int len)
{
	unsigned long pm;
	int en;

	ip1829drv_dbg("ip1829: +setMACReset...\n");
	if (sizeof(struct PortmapSetting) != len)
		return -EINVAL;

	pm = ((struct PortmapSetting *)cdata) ->portmap;
	en = ((struct PortmapSetting *)cdata) ->pmdata;
	if (pm & 0xE0000000)
		return -EINVAL;
	if (en != OP_FUNC_ENABLE)
		return -EINVAL;
	ip1829drv_dbg("pm=0x%08x\n", (unsigned int)pm);
	ip1829drv_dbg("en=%d\n", en);

	IP2Page(0);
	pm = ~pm;
	Write_Reg(P0REG_MACRESET, (u16)(pm & 0xFFFF));
	Write_Reg(P0REG_MACRESET+1, (u16)(pm >> 16));

	ip1829drv_dbg("ip1829: -setMACReset...\n");
	return 0;
}

int getMACReset(void *cdata, int len)
{
	u32 u32dat;

	ip1829drv_dbg("ip1829: +getMACReset...\n");
	if (sizeof(struct PortmapSetting) != len)
		return -EINVAL;

	IP2Page(0);
	u32dat = (u32)( (Read_Reg(P0REG_MACRESET+1) << 16) | Read_Reg(P0REG_MACRESET) );
	u32dat = ~u32dat;
	((struct PortmapSetting *)cdata) ->portmap = (u32dat & 0x1FFFFFFF);
	((struct PortmapSetting *)cdata) ->pmdata = OP_FUNC_ENABLE;
	ip1829drv_dbg("cdata ->portmap=0x%08x\n", (unsigned int)((struct PortmapSetting *)cdata) ->portmap);
	ip1829drv_dbg("cdata ->pmdata=%d\n", ((struct PortmapSetting *)cdata) ->pmdata);
	ip1829drv_dbg("ip1829: -getMACReset...\n");
	return 0;
}

int setMACSelfTestFunc(void *cdata, int len)
{
	unsigned long pm;
	int en;
	u32 u32dat;

	ip1829drv_dbg("ip1829: +setMACSelfTestFunc...\n");
	if (sizeof(struct PortmapSetting) != len)
		return -EINVAL;

	pm = ((struct PortmapSetting *)cdata) ->portmap;
	en = ((struct PortmapSetting *)cdata) ->pmdata;
	if (pm & 0xE0000000)
		return -EINVAL;
	if (en != OP_FUNC_ENABLE)
		return -EINVAL;
	ip1829drv_dbg("pm=0x%08x\n", (unsigned int)pm);
	ip1829drv_dbg("en=%d\n", en);

	IP2Page(0);
	u32dat = (u32)( (Read_Reg(P0REG_MACSELFPKT1) << 16) | Read_Reg(P0REG_MACSELFPKT) );
	u32dat &= 0xE0000000;
	u32dat |= pm;
	Write_Reg(P0REG_MACSELFPKT, (u16)(u32dat & 0xFFFF));
	Write_Reg(P0REG_MACSELFPKT1, (u16)(u32dat >> 16));

	ip1829drv_dbg("ip1829: -setMACSelfTestFunc...\n");
	return 0;
}

int getMACSelfTestFunc(void *cdata, int len)
{
	u32 u32dat;

	ip1829drv_dbg("ip1829: +getMACSelfTestFunc...\n");
	if (sizeof(struct PortmapSetting) != len)
		return -EINVAL;

	IP2Page(0);
	u32dat = (u32)( (Read_Reg(P0REG_MACSELFPKT1) << 16) | Read_Reg(P0REG_MACSELFPKT) );
	((struct PortmapSetting *)cdata) ->portmap = (u32dat & 0x1FFFFFFF);
	((struct PortmapSetting *)cdata) ->pmdata = OP_FUNC_ENABLE;
	ip1829drv_dbg("cdata ->portmap=0x%08x\n", (unsigned int)((struct PortmapSetting *)cdata) ->portmap);
	ip1829drv_dbg("cdata ->pmdata=%d\n", ((struct PortmapSetting *)cdata) ->pmdata);
	ip1829drv_dbg("ip1829: -getMACSelfTestFunc...\n");
	return 0;
}

int setMACSelfTestPktNum(void *cdata, int len)
{
	int pnum;

	ip1829drv_dbg("ip1829: +setMACSelfTestPktNum...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	pnum = ((struct GeneralSetting *)cdata) ->gdata;
	if (pnum!=OP_MAC_SELF_TEST_PKT_NO_16 && pnum!=OP_MAC_SELF_TEST_PKT_NO_256 &&
			pnum!=OP_MAC_SELF_TEST_PKT_NO_4096 && pnum!=OP_MAC_SELF_TEST_PKT_NO_32768)
		return -EINVAL;
	ip1829drv_dbg("pnum=%d\n", pnum);

	switch (pnum)
	{
		case OP_MAC_SELF_TEST_PKT_NO_16:
			pnum = 3;	break;

		case OP_MAC_SELF_TEST_PKT_NO_256:
			pnum = 2;	break;

		case OP_MAC_SELF_TEST_PKT_NO_4096:
			pnum = 1;	break;

		case OP_MAC_SELF_TEST_PKT_NO_32768:
			pnum = 0;	break;
	}
	_WriteRegBits(0, P0REG_MACSELFPKT1, 13, 2, pnum);
	ip1829drv_dbg("ip1829: -setMACSelfTestPktNum...\n");
	return 0;
}

int getMACSelfTestPktNum(void *cdata, int len)
{
	u16 u16dat;

	ip1829drv_dbg("ip1829: +getMACSelfTestPktNum...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	u16dat = (int)_ReadRegBits(0, P0REG_MACSELFPKT1, 13, 2);

	switch (u16dat)
	{
		case 0:
			u16dat = OP_MAC_SELF_TEST_PKT_NO_32768;		break;

		case 1:
			u16dat = OP_MAC_SELF_TEST_PKT_NO_4096;		break;

		case 2:
			u16dat = OP_MAC_SELF_TEST_PKT_NO_256;		break;

		case 3:
			u16dat = OP_MAC_SELF_TEST_PKT_NO_16;		break;
	}
	((struct GeneralSetting *)cdata) ->gdata = (int)u16dat;
	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	ip1829drv_dbg("ip1829: -getMACSelfTestPktNum...\n");
	return 0;
}

int getMACSelfTestResult(void *cdata, int len)
{
	int i;
	u32 u32dat;

	ip1829drv_dbg("ip1829: +getMACSelfTestResult...\n");
	if (sizeof(struct AllPortsSetting) != len)
		return -EINVAL;

	IP2Page(0);
	u32dat = (u32)( (Read_Reg(P0REG_MACSELF+1) << 16) | Read_Reg(P0REG_MACSELF) );
	ip1829drv_dbg("cdata ->apdata=\n[");
	for (i=0; i < SWITCH_MAX_IPORT_CNT; i++)
	{
		((struct AllPortsSetting *)cdata) ->apdata[i] = (int)(u32dat & 0x1);
		ip1829drv_dbg("%d, ", ((struct AllPortsSetting *)cdata) ->apdata[i]);
		if (i%5 == 4)
			ip1829drv_dbg("   ");
		if (i%10 == 9)
			ip1829drv_dbg("\n ");
		u32dat = u32dat >> 1;
	}
	ip1829drv_dbg("]\n");
	ip1829drv_dbg("ip1829: -getMACSelfTestResult...\n");
	return 0;
}

int setBpduCapMode(void *cdata, int len)
{
	int bcmode;

	ip1829drv_dbg("ip1829: +setBpduCapMode...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	bcmode = ((struct GeneralSetting *)cdata) ->gdata;
	if (bcmode!=OP_BPDU_CMODE_GLOBAL && bcmode!=OP_BPDU_CMODE_BY_PORT)
		return -EINVAL;
	ip1829drv_dbg("bcmode=%d\n", bcmode);

	_WriteRegBits(0, P0REG_BPDUPORTCAPCFG+3, 8, 1, bcmode);
	ip1829drv_dbg("ip1829: -setBpduCapMode...\n");
	return 0;
}

int getBpduCapMode(void *cdata, int len)
{
	int bcmode;

	ip1829drv_dbg("ip1829: +getBpduCapMode...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	bcmode = (int)_ReadRegBits(0, P0REG_BPDUPORTCAPCFG+3, 8, 1);

	((struct GeneralSetting *)cdata) ->gdata = bcmode;
	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	ip1829drv_dbg("ip1829: -getBpduCapMode...\n");
	return 0;
}

int setBpduPortAct(void *cdata, int len)
{
	int port, state;

	ip1829drv_dbg("ip1829: +setBpduPortAct...\n");
	if (sizeof(struct ByPortSetting) != len)
		return -EINVAL;

	port = ((struct ByPortSetting *)cdata) ->port;
	state= ((struct ByPortSetting *)cdata) ->pdata;
	if (port <= 0 || port >= SWITCH_MAX_IPORT_CNT)
		return -EINVAL;
	if (state!=OP_CAP_ACT_FORWARD && state!=OP_CAP_ACT_TO_CPU && state!=OP_CAP_ACT_DROP)
		return -EINVAL;
	ip1829drv_dbg("port=%d\n", port);
	ip1829drv_dbg("state=%d\n", state);

	_WriteRegBits(0, P0REG_BPDUPORTCAPCFG + (port-1)/8, ((port-1)%8)*2, 2, state);
	ip1829drv_dbg("ip1829: -setBpduPortAct...\n");
	return 0;
}

int getBpduPortAct(void *cdata, int len)
{
	int port, state;

	ip1829drv_dbg("ip1829: +getBpduPortAct...\n");
	if (sizeof(struct ByPortSetting) != len)
		return -EINVAL;

	port = ((struct ByPortSetting *)cdata) ->port;
	if (port <= 0 || port >= SWITCH_MAX_IPORT_CNT)
		return -EINVAL;
	ip1829drv_dbg("port=%d\n", port);

	state = (int)_ReadRegBits(0, P0REG_BPDUPORTCAPCFG + (port-1)/8, (port-1)%8*2, 2);

	ip1829drv_dbg("cdata ->pdata=%d\n", ((struct ByPortSetting *)cdata) ->pdata);
	ip1829drv_dbg("ip1829: -getBpduPortAct...\n");
	return 0;
}

int setStpPortState(void *cdata, int len)
{
	int fid, port, state;

	ip1829drv_dbg("ip1829: +setStpPortState...\n");
	if (sizeof(struct StpByFPSetting) != len)
		return -EINVAL;

	fid = ((struct StpByFPSetting *)cdata) ->fid;
	port= ((struct StpByFPSetting *)cdata) ->port;
	state=((struct StpByFPSetting *)cdata) ->pstate;
	if (fid < 1 || fid > 16)
		return -EINVAL;
	if (port <= 0 || port >= SWITCH_MAX_IPORT_CNT)
		return -EINVAL;
	if (state < OP_STP_STATE_DISCARD || state > OP_STP_STATE_FORWARD)
		return -EINVAL;
	ip1829drv_dbg("fid=%d\n", fid);
	ip1829drv_dbg("port=%d\n", port);
	ip1829drv_dbg("state=%d\n", state);

	_WriteRegBits(1, P1REG_SPANTREE + (fid-1)*4 + (port-1)/8, ((port-1)%8)*2, 2, state);
	ip1829drv_dbg("ip1829: -setStpPortState...\n");
	return 0;
}

int getStpPortState(void *cdata, int len)
{
	int fid, port, state;

	ip1829drv_dbg("ip1829: +getStpPortState...\n");
	if (sizeof(struct StpByFPSetting) != len)
		return -EINVAL;

	fid = ((struct StpByFPSetting *)cdata) ->fid;
	port= ((struct StpByFPSetting *)cdata) ->port;
	if (fid < 1 || fid > 16)
		return -EINVAL;
	if (port <= 0 || port >= SWITCH_MAX_IPORT_CNT)
		return -EINVAL;
	ip1829drv_dbg("fid=%d\n", fid);
	ip1829drv_dbg("port=%d\n", port);

	state = (int)_ReadRegBits(1, P1REG_SPANTREE + (fid-1)*4 + (port-1)/8, (port-1)%8*2, 2);

	((struct StpByFPSetting *)cdata) ->pstate = state;
	ip1829drv_dbg("cdata ->pstate=%d\n", ((struct StpByFPSetting *)cdata) ->pstate);
	ip1829drv_dbg("ip1829: -getStpPortState...\n");
	return 0;
}

int setStpAllPortsState(void *cdata, int len)
{
	int fid, state[SWITCH_MAX_IPORT_CNT-1], i;
	u8  u8dat;
	u16 u16dat=0;

	ip1829drv_dbg("ip1829: +setStpAllPortsState...\n");
	if (sizeof(struct StpAllPortsSetting) != len)
		return -EINVAL;

	fid = ((struct StpAllPortsSetting *)cdata) ->fid;
	if (fid < 1 || fid > 16)
		return -EINVAL;
	ip1829drv_dbg("fid=%d\n", fid);

	ip1829drv_dbg("state[]={ ");
	for (i=0; i < (SWITCH_MAX_IPORT_CNT-1); i++)
	{
		state[i] = ((struct StpAllPortsSetting *)cdata) ->pstate[i];
		if (state[i] < OP_STP_STATE_DISCARD || state[i] > OP_STP_STATE_FORWARD)
			return -EINVAL;
		ip1829drv_dbg("%d, ", state[i]);
	}
	ip1829drv_dbg("}");

	IP2Page(1);
	u8dat = (P1REG_SPANTREE + (fid-1)*4);
	for (i=0; i < (SWITCH_MAX_IPORT_CNT-1); i++)
	{
		if ((i & 0x7) == 0)
		{
			u16dat = 0;
		}
		u16dat |= (u16)( state[i] << (i%8*2) );
		if (i == (SWITCH_MAX_IPORT_CNT-2))	// CPU port is always in forwarding state
			u16dat |= (u16)(OP_STP_STATE_FORWARD << 8);
		if (i%8==7 || i==(SWITCH_MAX_IPORT_CNT-2))
		{
			Write_Reg(u8dat, u16dat);
			u8dat++;
		}
	}
	ip1829drv_dbg("ip1829: -setStpAllPortsState...\n");
	return 0;
}

int getStpAllPortsState(void *cdata, int len)
{
	int fid, i, j;
	u8  u8dat;
	u16 u16dat;

	ip1829drv_dbg("ip1829: +getStpAllPortsState...\n");
	if (sizeof(struct StpAllPortsSetting) != len)
		return -EINVAL;

	fid = ((struct StpAllPortsSetting *)cdata) ->fid;
	if (fid < 1 || fid > 16)
		return -EINVAL;
	ip1829drv_dbg("fid=%d\n", fid);

	IP2Page(1);
	u8dat = (P1REG_SPANTREE + (fid-1)*4);
	for (i=0; i < 4; i++)
	{
		u16dat= Read_Reg(u8dat+i);
		ip1829drv_dbg("cdata ->pstate=");
		for (j=0; j < 8; j++)
		{
			if ((i*8+j) == 28)	break;
			((struct StpAllPortsSetting *)cdata) ->pstate[i*8+j] =
				(int)( (u16dat >> (j*2)) & 0x3);
			ip1829drv_dbg("%d, ",
					((struct StpAllPortsSetting *)cdata) ->pstate[i*8+j]);
		}
		ip1829drv_dbg("\n");
	}
	ip1829drv_dbg("ip1829: -getStpAllPortsState...\n");
	return 0;
}

int setTrunkHashMthdSeq(void *cdata, int len)
{
	int en;

	ip1829drv_dbg("ip1829: +setTrunkHashMthdSeq...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	en = ((struct GeneralSetting *)cdata) ->gdata;
	if (en!=OP_FUNC_DISABLE && en!=OP_FUNC_ENABLE)
		return -EINVAL;
	ip1829drv_dbg("en=%d\n", en);

	_WriteRegBits(1, P1REG_TRUNKCFG, 3, 1, en);
	ip1829drv_dbg("ip1829: -setTrunkHashMthdSeq...\n");
	return 0;
}

int getTrunkHashMthdSeq(void *cdata, int len)
{
	int en;

	ip1829drv_dbg("ip1829: +getTrunkHashMthdSeq...\n");
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	en = (int)_ReadRegBits(1, P1REG_TRUNKCFG, 3, 1);

	((struct GeneralSetting *)cdata) ->gdata = en;
	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	ip1829drv_dbg("ip1829: -getTrunkHashMthdSeq...\n");
	return 0;
}

int setTrunkGrpCombine(void *cdata, int len)
{
	int grps, en;

	ip1829drv_dbg("ip1829: +setTrunkGrpCombine...\n");
	if (sizeof(struct TrunkCombineSetting) != len)
		return -EINVAL;

	grps=((struct TrunkCombineSetting *)cdata) ->tgrps;
	en = ((struct TrunkCombineSetting *)cdata) ->cen;
	if (grps < OP_TRUNK_COMBINE_G1_G2 || grps > OP_TRUNK_COMBINE_G5_G6)
		return -EINVAL;
	if (en!=OP_FUNC_DISABLE && en!=OP_FUNC_ENABLE)
		return -EINVAL;
	ip1829drv_dbg("grps=%d\n", grps);
	ip1829drv_dbg("en=%d\n", en);

	_WriteRegBits(1, P1REG_TRUNKCFG, grps, 1, en);
	ip1829drv_dbg("ip1829: -setTrunkGrpCombine...\n");
	return 0;
}

int getTrunkGrpCombine(void *cdata, int len)
{
	int grps, en;

	ip1829drv_dbg("ip1829: +getTrunkGrpCombine...\n");
	if (sizeof(struct TrunkCombineSetting) != len)
		return -EINVAL;

	grps=((struct TrunkCombineSetting *)cdata) ->tgrps;
	if (grps < OP_TRUNK_COMBINE_G1_G2 || grps > OP_TRUNK_COMBINE_G5_G6)
		return -EINVAL;
	ip1829drv_dbg("grps=%d\n", grps);

	en = (int)_ReadRegBits(1, P1REG_TRUNKCFG, grps, 1);

	((struct TrunkCombineSetting *)cdata) ->cen = en;
	ip1829drv_dbg("cdata ->cen=%d\n", ((struct TrunkCombineSetting *)cdata) ->cen);
	ip1829drv_dbg("ip1829: -getTrunkGrpCombine...\n");
	return 0;
}

//------------ LUT functions:common ----------------------

int setLutAgingTime(void *cdata, int len)
{
	int time;

	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	time = ((struct GeneralSetting *)cdata) ->gdata;
	if (time > 0x1800000)
		return -EINVAL;
	ip1829drv_dbg("time=0x%08x\n", time);

	_WriteRegBits(1, P1REG_LUTAGINGTIME, 0, 15, (u16)time/AGING_TIME_UNIT);
	FUNC_MSG_OUT;
	return 0;
}

int getLutAgingTime(void *cdata, int len)
{
	int time;

	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	time = (int)_ReadRegBits(1, P1REG_LUTAGINGTIME, 0, 15)*AGING_TIME_UNIT;

	((struct GeneralSetting *)cdata) ->gdata = time;

	ip1829drv_dbg("cdata ->gdata=0x%08x\n", ((struct GeneralSetting *)cdata) ->gdata);
	FUNC_MSG_OUT;
	return 0;
}

int setLutAgingTimeEnable(void *cdata, int len)
{
	int en;

	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	en = ((struct GeneralSetting *)cdata) ->gdata;
	if ( !(en==OP_FUNC_DISABLE || en==OP_FUNC_ENABLE) )
		return -EINVAL;

	ip1829drv_dbg("en=%d\n", en);

	_WriteRegBits(1, P1REG_LUTAGINGTIME, 15, 1, (en==OP_FUNC_ENABLE)?0:1);
	FUNC_MSG_OUT;
	return 0;
}

int getLutAgingTimeEnable(void *cdata, int len)
{
	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	((struct GeneralSetting *)cdata) ->gdata = (int)_ReadRegBits(1, P1REG_LUTAGINGTIME, 15, 1)?OP_FUNC_DISABLE:OP_FUNC_ENABLE;

	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	FUNC_MSG_OUT;
	return 0;
}

int setLutLearningNullSA(void *cdata, int len)
{
	int en;

	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	en = ((struct GeneralSetting *)cdata) ->gdata;
	if ( !(en==OP_FUNC_DISABLE || en==OP_FUNC_ENABLE) )
		return -EINVAL;

	ip1829drv_dbg("en=%d\n", en);

	_WriteRegBits(1, P1REG_SRCLEARNCFG, 2, 1, en);
	FUNC_MSG_OUT;
	return 0;
}

int getLutLearningNullSA(void *cdata, int len)
{
	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	((struct GeneralSetting *)cdata) ->gdata = (int)_ReadRegBits(1, P1REG_SRCLEARNCFG, 2, 1)?OP_FUNC_ENABLE:OP_FUNC_DISABLE;

	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	FUNC_MSG_OUT;
	return 0;
}

int setLutHashingAlgorithm(void *cdata, int len)
{
	int hash;

	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	hash = ((struct GeneralSetting *)cdata) ->gdata;
	if ( !(hash==OP_HASH_DIRECT || hash==OP_HASH_CRC) )
		return -EINVAL;

	ip1829drv_dbg("hash=%d\n", hash);

	_WriteRegBits(1, P1REG_SRCLEARNCFG, 3, 1, (hash==OP_HASH_CRC)?1:0);
	FUNC_MSG_OUT;
	return 0;
}

int getLutHashingAlgorithm(void *cdata, int len)
{
	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	((struct GeneralSetting *)cdata) ->gdata = _ReadRegBits(1, P1REG_SRCLEARNCFG, 3, 1)?OP_HASH_CRC:OP_HASH_DIRECT;

	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	FUNC_MSG_OUT;
	return 0;
}

int setLutBindingEnable(void *cdata, int len)
{
	int en;

	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	en = ((struct GeneralSetting *)cdata) ->gdata;
	if ( !(en==OP_FUNC_DISABLE || en==OP_FUNC_ENABLE) )
		return -EINVAL;

	ip1829drv_dbg("en=%d\n", en);

	_WriteRegBits(1, P1REG_SRCLEARNCFG, 5, 1, (en==OP_FUNC_ENABLE)?0:1);
	_WriteRegBits(1, P1REG_SRCLEARNCFG, 6, 1, (en==OP_FUNC_ENABLE)?1:0);

	FUNC_MSG_OUT;
	return 0;
}

int getLutBindingEnable(void *cdata, int len)
{
	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	((struct GeneralSetting *)cdata) ->gdata = (int)(_ReadRegBits(1, P1REG_SRCLEARNCFG, 5, 2)==0x2?OP_FUNC_ENABLE:OP_FUNC_DISABLE);

	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	FUNC_MSG_OUT;
	return 0;
}


int setLutLearnPktDropByVlanIgsChk(void *cdata, int len)
{
	int en;

	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	en = ((struct GeneralSetting *)cdata) ->gdata;
	if ( !(en==OP_FUNC_DISABLE || en==OP_FUNC_ENABLE) )
		return -EINVAL;

	ip1829drv_dbg("en=%d\n", en);

	_WriteRegBits(1, P1REG_SRCLEARNCFG, 7, 1, en);
	FUNC_MSG_OUT;
	return 0;
}

int getLutLearnPktDropByVlanIgsChk(void *cdata, int len)
{
	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	((struct GeneralSetting *)cdata) ->gdata = (int)_ReadRegBits(1, P1REG_SRCLEARNCFG, 7, 1)?OP_FUNC_ENABLE:OP_FUNC_DISABLE;

	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	FUNC_MSG_OUT;
	return 0;
}


//------------ LUT functions:common end ----------------------

//------------ LUT functions:ip1829 ------------------------------

int setLutLearningMode(void *cdata, int len)
{
	int mode;

	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	mode = ((struct GeneralSetting *)cdata) ->gdata;
	if ( !(	mode==OP_LUT_LEARN_MODE_ALL_BY_AGING_TIME ||
				mode==OP_LUT_LEARN_MODE_NEVER_OVERWRITE ||
				mode==OP_LUT_LEARN_MODE_L2_BY_AGING_TIME) )
		return -EINVAL;

	ip1829drv_dbg("mode=%d\n", mode);

	_WriteRegBits(1, P1REG_SRCLEARNCFG,0, 2, mode);
	FUNC_MSG_OUT;
	return 0;
}

int getLutLearningMode(void *cdata, int len)
{
	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	((struct GeneralSetting *)cdata) ->gdata = (int)_ReadRegBits(1, P1REG_SRCLEARNCFG, 0, 2);

	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	FUNC_MSG_OUT;
	return 0;
}


int setLutUnknownSARule(void *cdata, int len)
{
	int rule;

	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	rule = ((struct GeneralSetting *)cdata) ->gdata;
	if ( !(	rule==OP_LUT_UNKNOWNU_SA_DROP ||
				rule==OP_LUT_UNKNOWNU_SA_FWD_2_CPU ||
				rule==OP_LUT_UNKNOWN_SA_FWD) )
		return -EINVAL;

	ip1829drv_dbg("rule=%d\n", rule);

	_WriteRegBits(1, P1REG_SRCLEARNCFG, 4, 2, rule);
	FUNC_MSG_OUT;
	return 0;
}

int getLutUnknownSARule(void *cdata, int len)
{
	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	((struct GeneralSetting *)cdata) ->gdata = (int)_ReadRegBits(1, P1REG_SRCLEARNCFG, 4, 2);

	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	FUNC_MSG_OUT;
	return 0;
}

int setLutEntry(void *cdata, int len)
{
	int i;
	unsigned char f_notMatch = 0;
	u16 u16dat, index;
	struct IP1829LUTSetting *luts;

	FUNC_MSG_IN;
	if (sizeof(struct IP1829LUTSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	luts = (struct IP1829LUTSetting *)cdata;

	if(luts ->action!=OP_ENTRY_CREATE && luts ->action!=OP_ENTRY_CONFIG && luts ->action!=OP_ENTRY_DELETE)
	{
		ip1829drv_dbg("action error:%d\n", luts ->action);
		return -EINVAL;
	}
	if(luts ->block > 2)
	{
		ip1829drv_dbg("block error:%d\n", luts ->block);
		return -EINVAL;
	}
	if(luts ->action!=OP_ENTRY_DELETE)
	{
		if(luts ->entry.fid > 15)
		{
			ip1829drv_dbg("fid error:%d\n", luts ->entry.fid);
			return -EINVAL;
		}
		if(luts ->entry.srcport > 32)
		{
			ip1829drv_dbg("srcport error:%d\n", luts ->entry.srcport);
			return -EINVAL;
		}
		if(luts ->entry.aging > 15)
		{
			ip1829drv_dbg("aging error:%d\n", luts ->entry.aging);
			return -EINVAL;
		}
		if(luts ->entry.priority > 8)
		{
			ip1829drv_dbg("priority error:%d\n", luts ->entry.priority);
			return -EINVAL;
		}
	}

	/* initialize retval value */
	luts ->retval = 0;
	/* calculate hash value */
	index = 0;//LUT_hash(luts ->entry.mac, luts ->entry.fid);

	ip1829drv_dbg("index=%d\n", index);
	ip1829drv_dbg("block=%d\n", luts ->block);
	ip1829drv_dbg("MAC=\n");
	for(i=0; i<6; i++)
		ip1829drv_dbg("[%02X]\n", luts ->entry.mac[i]);
	ip1829drv_dbg("fid=%d\n", luts ->entry.fid);
	ip1829drv_dbg("srcport=%d\n", luts ->entry.srcport);
	ip1829drv_dbg("aging=%d\n", luts ->entry.aging);
	ip1829drv_dbg("priority=%d\n", luts ->entry.priority);
	ip1829drv_dbg("pri_en=%d\n", luts ->entry.flag.pri_en);
	ip1829drv_dbg("drop=%d\n", luts ->entry.flag.drop);
	ip1829drv_dbg("snif1=%d\n", luts ->entry.flag.snif1);
	ip1829drv_dbg("snif2=%d\n", luts ->entry.flag.snif2);
	ip1829drv_dbg("sflow=%d\n", luts ->entry.flag.sflow);

	IP2Page(1);

	/* set mac & fid & hw_calc let switch calculate index */
	/* copy mac to register from cdata */
	for(i=0; i<3; i++)
	{
		u16dat = (u16)(luts ->entry.mac[4 - i*2])<<8 | (u16)(luts ->entry.mac[5 - i*2]);
		Write_Reg(P1REG_LUTDATA+i, u16dat );
	}

	/* set fid to register from cdata */
	u16dat = luts ->entry.fid & 0x000f;
	Write_Reg(P1REG_LUTDATA_3, u16dat );

	/* setup table address access method by HW */
	Write_Reg(P1REG_LUTDATA_4, 0x0020 );

	/* read entry of target index from MAC table to check */
	u16dat = index;				//set index
	u16dat |= (u16)luts ->block<<13;	//set block num
	u16dat |= (u16)0x2<<14;		//set read command & command trigger
	Write_Reg(P1REG_LUTCFG, u16dat );

	/* need to check does trigger bit has been put down */
	while((Read_Reg(P1REG_LUTCFG) >> 15)&0x1);

	/* check if register value is same with input data */
	for(i=0; i<3; i++)
	{
		u16dat = Read_Reg(P1REG_LUTDATA+i);
		if( luts ->entry.mac[5 - i*2] != (u8)(u16dat&0xFF) )
		{
			ip1829drv_dbg("MAC isn't matched!\n");
			ip1829drv_dbg("luts ->entry.mac[%d]=%02X, reg=%02x\n", (5-i*2), luts ->entry.mac[5 - i*2], (u16dat>>8)&0xFF);
			f_notMatch = 1;
		}
		if( luts ->entry.mac[4 - i*2] != (u8)((u16dat>>8)&0xFF) )
		{
			ip1829drv_dbg("MAC isn't matched!\n");
			ip1829drv_dbg("luts ->entry.mac[%d]=%02X, reg=%02x\n", (4-i*2), luts ->entry.mac[4 - i*2], (u16dat>>8)&0xFF);
			f_notMatch = 1;
		}
	}

	/* check if fid is matched to input cdata */
	u16dat = Read_Reg(P1REG_LUTDATA_3);
	ip1829drv_dbg("e4=%04x\n", u16dat);
	if( luts ->entry.fid != (u16dat & 0x000f))
	{
		ip1829drv_dbg("fid is not match\n");
		ip1829drv_dbg("luts ->entry.fid=%d, reg=%d\n", luts ->entry.fid, (u16dat & 0x000f));
		f_notMatch = 1;
	}

	/* read register to check aging time */
	u16dat = Read_Reg(P1REG_LUTDATA_4);
	if(luts ->action==OP_ENTRY_CREATE)
	{
		ip1829drv_dbg("OP_ENTRY_CREATE\n");
		if((u16dat & 0x000f) == 0xf)
		{
			ip1829drv_dbg("target entry has existed!!\n");
			u16dat = Read_Reg(P1REG_LUTDATA_0);
			u16dat = Read_Reg(P1REG_LUTDATA_1);
			u16dat = Read_Reg(P1REG_LUTDATA_2);
			luts ->retval = OP_ENTRY_EXISTS;
			return 0;
		}
		for(i=0; i<3; i++)
		{
			u16dat = (u16)(luts ->entry.mac[4 - i*2])<<8 | (u16)(luts ->entry.mac[5 - i*2]);
			Write_Reg(P1REG_LUTDATA+i, u16dat );
		}
		u16dat = 0;
		u16dat = luts ->entry.fid;
		u16dat |= (u16)luts ->entry.priority<<4;
		u16dat |= (u16)luts ->entry.flag.drop<<7;
		u16dat |= (u16)luts ->entry.flag.snif1<<8;
		u16dat |= (u16)luts ->entry.flag.snif2<<9;
		u16dat |= (u16)luts ->entry.flag.sflow<<10;
		u16dat |= (u16)(luts ->entry.srcport - 1)<<11;
		Write_Reg(P1REG_LUTDATA_3, u16dat );

		u16dat = 0x0f;	//set aging to static
		u16dat |= luts ->entry.flag.pri_en<<4;
		u16dat |= 0x01<<5;//hw_calc
		Write_Reg(P1REG_LUTDATA_4, u16dat );

		u16dat = 0;					//set index
		u16dat |= (u16)luts ->block<<13;	//set block num
		u16dat |= (u16)0x3<<14;		//set write command & command trigger
		Write_Reg(P1REG_LUTCFG, u16dat );

		/* need to check does trigger bit has been put down */
		while((Read_Reg(P1REG_LUTCFG) >> 15)&0x1);
	}
	else if(luts ->action==OP_ENTRY_CONFIG)
	{
		ip1829drv_dbg("OP_ENTRY_CONFIG\n");
		/* if MAC or FID isn't match, we should abort this configuration */
		if(f_notMatch)
		{
			ip1829drv_dbg("MAC or FID not match!!\n");
			luts ->retval = OP_ENTRY_NOT_MATCH;
			return 0;
		}

		/* check if aging is invalid entry, invalid entry conldn't be config */
		if((u16dat & 0x000f) == 0x0)
		{
			ip1829drv_dbg("aging value error!!\n");
			luts ->retval = OP_ENTRY_NOT_FOUND;
			return 0;
		}

		u16dat = luts ->entry.fid;
		u16dat |= (u16)luts ->entry.priority<<4;
		u16dat |= (u16)luts ->entry.flag.drop<<7;
		u16dat |= (u16)luts ->entry.flag.snif1<<8;
		u16dat |= (u16)luts ->entry.flag.snif2<<9;
		u16dat |= (u16)luts ->entry.flag.sflow<<10;
		u16dat |= (u16)(luts ->entry.srcport - 1)<<11;
		Write_Reg(P1REG_LUTDATA_3, u16dat );

		u16dat = luts ->entry.aging;
		u16dat |= luts ->entry.flag.pri_en<<4;
		u16dat |= 0x01<<5;//hw_calc
		Write_Reg(P1REG_LUTDATA_4, u16dat );

		u16dat = 0;					//set index
		u16dat |= (u16)luts ->block<<13;	//set block num
		u16dat |= (u16)0x3<<14;		//set write command & command trigger
		Write_Reg(P1REG_LUTCFG, u16dat );

		/* need to check does trigger bit has been put down */
		while((Read_Reg(P1REG_LUTCFG) >> 15)&0x1);
	}
	else if(luts ->action==OP_ENTRY_DELETE)
	{
		ip1829drv_dbg("OP_ENTRY_DELETE\n");
		if(f_notMatch)
		{
			ip1829drv_dbg("MAC or FID not match!!\n");
			luts ->retval = OP_ENTRY_NOT_MATCH;
			return 0;
		}
		if((u16dat & 0x000f)!=0xf)
		{
			/* only static entry can be deleted */
			ip1829drv_dbg("not static entry!!\n");
			luts ->retval = OP_ENTRY_NOT_FOUND;
			return 0;
		}

		u16dat = 0x0;	//set aging to invalid
		u16dat |= luts ->entry.flag.pri_en;
		u16dat |= 0x01<<5;//hw_calc
		Write_Reg(P1REG_LUTDATA_4, u16dat );

		u16dat = 0;					//set index
		u16dat |= (u16)luts ->block<<13;	//set block num
		u16dat |= (u16)0x3<<14;		//set write command & command trigger
		Write_Reg(P1REG_LUTCFG, u16dat );

		/* need to check does trigger bit has been put down */
		while((Read_Reg(P1REG_LUTCFG) >> 15)&0x1);
	}
	FUNC_MSG_OUT;
	return 0;
}

int getLutEntry(void *cdata, const int len)
{
	int i;
	bool f_notMatch = 0;
	u16 u16dat, u16_e4, u16_e5;
	struct IP1829LUTSetting *luts;

	FUNC_MSG_IN;
	/* check cdata length */
	if (sizeof(struct IP1829LUTSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	luts = (struct IP1829LUTSetting *)cdata;

	/* check block range */
	if(luts -> block >= MAX_BLOCK_NUM)
	{
		ip1829drv_dbg("Error: block=%d\n", luts ->block);
		return -EINVAL;
	}

	/* check index range */
	if(luts ->index >= MAX_LUT_ENTRY_NUM)
	{
		ip1829drv_dbg("Error: index=%d\n", luts ->index);
		return -EINVAL;
	}

	/* set to register page 1 */
	IP2Page(1);

	/* check command action */
	if(luts ->action == OP_ENTRY_GET_BY_INFO)
	{
		if(luts ->entry.fid >= MAX_FID_NUM)
		{
			ip1829drv_dbg("Error: fid=%d\n", luts ->entry.fid);
			return -EINVAL;
		}

		/* copy mac to register from cdata */
		for(i=0; i<3; i++)
		{
			u16dat = (u16)(luts ->entry.mac[4 - i*2])<<8 | (u16)(luts ->entry.mac[5 - i*2]);
			Write_Reg(P1REG_LUTDATA+i, u16dat );
		}

		/* set fid to register from cdata */
		Write_Reg(P1REG_LUTDATA_3, luts ->entry.fid );

		/* setup table address access method by HW */
		Write_Reg(P1REG_LUTDATA_4, 0x0020 );
	}
	else if(luts ->action == OP_ENTRY_GET_BY_INDEX)
	{
		/* setup table address access method by CPU */
		Write_Reg(P1REG_LUTDATA_4, 0 );
	}
	else
	{
		ip1829drv_dbg("Error: action=%d\n", luts ->action);
		return -EINVAL;
	}

	/* initialize return value */
	luts ->retval = 0;

	u16dat = luts ->index;		//set index
	u16dat |= (u16)(luts ->block)<<13;//set block num
	u16dat |= (u16)0x2<<14;			//set read command & command trigger
	Write_Reg(P1REG_LUTCFG, u16dat );

	/* check does trigger bit has been pull down */
	while(Read_Reg(P1REG_LUTCFG)&0x8000);

	/* catch aging time to check entry state */
	u16_e5 = Read_Reg(P1REG_LUTDATA_4);
	if((u16_e5 & 0x000f) == 0x0)
	{
		luts ->retval = OP_ENTRY_NOT_FOUND;
		return 0;
	}

	if(luts ->action == OP_ENTRY_GET_BY_INFO)
	{
		/* if action is get by entry info, check if register value is same with input data */
		/* catch MAC address */
		for(i=0; i<3; i++)
		{
			u16dat = Read_Reg(P1REG_LUTDATA+i);
			if( luts ->entry.mac[5 - i*2] != (u8)(u16dat&0xFF) )
				f_notMatch = 1;
			if( luts ->entry.mac[4 - i*2] != (u8)((u16dat>>8)&0xFF) )
				f_notMatch = 1;
		}

		/* catch other info. */
		u16_e4 = Read_Reg(P1REG_LUTDATA_3);
		/* check weather fid is matched to input cdata */
		if( luts ->entry.fid != (u16_e4& 0x000f))
			f_notMatch = 1;

		if((u16_e5 & 0x000f) == 0xf)
		{
			if(f_notMatch)
			{
				ip1829drv_dbg("Error: MAC or FID is not match\n");
				luts ->retval = OP_ENTRY_NOT_MATCH;
				return 0;
			}
			else
				luts ->retval = OP_ENTRY_EXISTS;
		}

	}
	else//if(luts ->action == OP_ENTRY_GET_BY_INDEX)
	{
		if((u16_e5 & 0x000f) == 0xf)
			luts ->retval = OP_ENTRY_EXISTS;

		/* if action is get by entry index, copy the register value into output data */
		/* catch MAC address */
		for(i=0; i<3; i++)
		{
			u16dat = Read_Reg(P1REG_LUTDATA+i);
			luts ->entry.mac[5 - i*2] = (u8)(u16dat&0xFF);
			luts ->entry.mac[4 - i*2] = (u8)((u16dat>>8)&0xFF);
		}

		u16_e4 = Read_Reg(P1REG_LUTDATA_3);
		luts ->entry.fid	= u16_e4 & 0x000f;
	}

	luts ->entry.priority	= (u16_e4>>4) & 0x0007;
	luts ->entry.flag.drop	= (u16_e4>>7) & 0x0001;
	luts ->entry.flag.snif1	= (u16_e4>>8) & 0x0001;
	luts ->entry.flag.snif2	= (u16_e4>>9) & 0x0001;
	luts ->entry.flag.sflow	= (u16_e4>>10) & 0x0001;
	luts ->entry.srcport	= ((u16_e4>>11) & 0x001f)+1;

	luts ->entry.aging	= u16_e5 & 0x000f;
	luts ->entry.flag.pri_en= ((u16_e5 >>4) & 0x0001);

	ip1829drv_dbg("cdata ->MAC=");
	for(i=0; i<6; i++)
		ip1829drv_dbg("[%02X]", luts ->entry.mac[i]);
	ip1829drv_dbg("\ncdata ->fid=%d\n", luts ->entry.fid);
	ip1829drv_dbg("cdata ->srcport=%d\n", luts ->entry.srcport);
	ip1829drv_dbg("cdata ->aging=%d\n", luts ->entry.aging);
	ip1829drv_dbg("cdata ->priority=%d\n", luts ->entry.priority);
	ip1829drv_dbg("cdata ->pri_en=%d\n", luts ->entry.flag.pri_en);
	ip1829drv_dbg("cdata ->drop=%d\n", luts ->entry.flag.drop);
	ip1829drv_dbg("cdata ->snif1=%d\n", luts ->entry.flag.snif1);
	ip1829drv_dbg("cdata ->snif2=%d\n", luts ->entry.flag.snif2);
	ip1829drv_dbg("cdata ->sflow=%d\n", luts ->entry.flag.sflow);
	FUNC_MSG_OUT;
	return 0;
}

/* used by scaning MAC, so make it faster as soon as possible */
int getLutValidEntry(void *cdata, int len)
{
	int i;
	u16 u16dat, u16_e4, u16_e5;
	unsigned short index;
	unsigned char block;
	struct IP1829LUTSetting *luts;
	FUNC_MSG_IN;

	luts = (struct IP1829LUTSetting *)cdata;

	/* check block range */
	if(luts -> block >= MAX_BLOCK_NUM)
	{
		ip1829drv_dbg("Error: block=%d\n", luts ->block);
		return -EINVAL;
	}

	/* check index range */
	if(luts ->index >= MAX_LUT_ENTRY_NUM)
	{
		ip1829drv_dbg("Error: index=%d\n", luts ->index);
		return -EINVAL;
	}
	index = luts ->index;

	/* set to register page 1 */
	IP2Page(1);

	/* setup table address access method by CPU */
	Write_Reg(P1REG_LUTDATA_4, 0 );

	/* initialize return value */
	luts ->retval = 0;
	for(block = luts -> block; block<MAX_BLOCK_NUM; block++)
	{
		for(; index < MAX_LUT_ENTRY_NUM; index++)
		{
			u16dat = index;				//set index
			u16dat |= (u16)(block)<<13;	//set block num
			u16dat |= (u16)0x2<<14;		//set read command & command trigger
			Write_Reg(P1REG_LUTCFG, u16dat );

			/* check does trigger bit has been pull down */
			while(Read_Reg(P1REG_LUTCFG)&0x8000);

			/* catch aging time to check entry state */
			u16_e5 = Read_Reg(P1REG_LUTDATA_4);
			if((u16_e5 &0x000f) == 0x0)
				continue;
			if((u16_e5 &0x000f) == 0xf)
				continue;

			/* check weather source port bigger than CPU port num */
			u16_e4 = Read_Reg(P1REG_LUTDATA_3);
			if((((u16_e4>>11) & 0x001f) + 1) > SWITCH_MAX_IPORT_CNT)
				continue;

			/* check weather this entry is in target portlist */
			if(!((1UL << ((u16_e4>>11) & 0x001f))&luts ->tarports))
				continue;

			/* if find a valid entry, copy the register value into output data */
			/* catch MAC address */
			for(i=0; i<3; i++)
			{
				u16dat = Read_Reg(P1REG_LUTDATA+i);
				luts ->entry.mac[5 - i*2] = (u8)(u16dat&0xFF);
				luts ->entry.mac[4 - i*2] = (u8)((u16dat>>8)&0xFF);
			}
			luts ->entry.priority	= (u16_e4>>4) & 0x0007;
			luts ->entry.flag.drop	= (u16_e4>>7) & 0x0001;
			luts ->entry.flag.snif1	= (u16_e4>>8) & 0x0001;
			luts ->entry.flag.snif2	= (u16_e4>>9) & 0x0001;
			luts ->entry.flag.sflow	= (u16_e4>>10) & 0x0001;
			luts ->entry.srcport	= ((u16_e4>>11) & 0x001f)+1;

			luts ->entry.aging	= u16_e5 & 0x000f;
			luts ->entry.flag.pri_en= ((u16_e5 >>4) & 0x0001);

			luts ->index = index;
			luts ->block = block;
			luts ->retval = OP_ENTRY_EXISTS;
			ip1829drv_dbg("=====================================\n");
			ip1829drv_dbg("index=%d\n", luts ->index);
			ip1829drv_dbg("block=%d\n", luts ->block);
			ip1829drv_dbg("retval=%lX\n", luts ->retval);
			ip1829drv_dbg("MAC=");
			for(i=0; i<6; i++)
				ip1829drv_dbg("[%02X]", luts ->entry.mac[i]);
			ip1829drv_dbg("\nfid=%d\n", luts ->entry.fid);
			ip1829drv_dbg("srcport=%d\n", luts ->entry.srcport);
			ip1829drv_dbg("aging=%d\n", luts ->entry.aging);
			ip1829drv_dbg("priority=%d\n", luts ->entry.priority);
			ip1829drv_dbg("pri_en=%d\n", luts ->entry.flag.pri_en);
			ip1829drv_dbg("drop=%d\n", luts ->entry.flag.drop);
			ip1829drv_dbg("snif1=%d\n", luts ->entry.flag.snif1);
			ip1829drv_dbg("snif2=%d\n", luts ->entry.flag.snif2);
			ip1829drv_dbg("sflow=%d\n", luts ->entry.flag.sflow);
			goto out_getLutValidEntry;
		}
		index = 0;
	}

out_getLutValidEntry:
	FUNC_MSG_OUT;
	return 0;
}
//------------ LUT functions:ip1829 end -------------------------

//------------ IGMP functions:ip1829 ----------------------------
int setIGMPSnooping(void *cdata, int len){
	struct GeneralSetting *igmp;

	ip1829drv_dbg("In setIGMPSnoopin.\n");
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("length error!\n");
		return -EINVAL;
	}

	igmp = (struct GeneralSetting *)cdata;

	switch(igmp ->gdata){
		case OP_FUNC_ENABLE:
			ip1829drv_dbg("Enable IGMP\n");
			break;
		case OP_FUNC_DISABLE:
			ip1829drv_dbg("Disable IGMP\n");
			break;
		default:
			ip1829drv_dbg("Option can't find\n");
			return -EINVAL;
	}
	ip1829drv_dbg("Set IGMPSNOP: %x\n",igmp ->gdata);
	_WriteRegBits(1, P1REG_IGMPSNOP, 0, 1, igmp ->gdata);
	return 0;
}

int getIGMPSnooping(void *cdata, int len){
	struct GeneralSetting *igmp;

	ip1829drv_dbg("In getIGMPSnoopin.\n");
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("length error!\n");
		return -EINVAL;
	}

	igmp = (struct GeneralSetting *)cdata;

	igmp ->gdata = (int)_ReadRegBits(1, P1REG_IGMPSNOP, 0, 1)?OP_FUNC_ENABLE:OP_FUNC_DISABLE;
	ip1829drv_dbg("IGMP %s",(igmp ->gdata==OP_FUNC_ENABLE)?"ENABLE":"DISABLE");
	return 0;
}

int setIGMPMctByCPU(void *cdata, int len){
	struct GeneralSetting *igmp;

	ip1829drv_dbg("In setIGMPMctByCPU.\n");
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("length error!\n");
		return -EINVAL;
	}

	igmp = (struct GeneralSetting *)cdata;

	switch(igmp ->gdata){
		case OP_FUNC_ENABLE:
			ip1829drv_dbg("Enable MCT By CPU\n");
			break;
		case OP_FUNC_DISABLE:
			ip1829drv_dbg("Disable MCT By CPU\n");
			break;
		default:
			ip1829drv_dbg("Option can't find\n");
			return -EINVAL;
	}
	ip1829drv_dbg("Set IGMPSNOP: %x\n", igmp ->gdata);
	_WriteRegBits(1, P1REG_IGMPSNOP, 1, 1, igmp ->gdata);
	return 0;
}

int getIGMPMctByCPU(void *cdata, int len){
	struct GeneralSetting *igmp;

	ip1829drv_dbg("In getIGMPMctByCPU.\n");
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("length error!\n");
		return -EINVAL;
	}

	igmp = (struct GeneralSetting *)cdata;

	igmp ->gdata = (int)_ReadRegBits(1, P1REG_IGMPSNOP, 1, 1)?OP_FUNC_ENABLE:OP_FUNC_DISABLE;

	ip1829drv_dbg("IGMPMctByCPU %s",(igmp ->gdata==OP_FUNC_ENABLE)?"ENABLE":"DISABLE");
	return 0;
}

int setIGMPGroupAgain(void *cdata, int len){
	struct GeneralSetting *igmp;

	ip1829drv_dbg("In setIGMPGroupAgain.\n");
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("length error!\n");
		return -EINVAL;
	}

	igmp = (struct GeneralSetting *)cdata;

	switch(igmp ->gdata){
		case OP_FUNC_ENABLE:
			ip1829drv_dbg("Enable Group Again\n");
			break;
		case OP_FUNC_DISABLE:
			ip1829drv_dbg("Disable Group Again\n");
			break;
		default:
			ip1829drv_dbg("Option can't find\n");
			return -EINVAL;
	}
	ip1829drv_dbg("Set IGMPSNOP: %x\n",igmp ->gdata);
	_WriteRegBits(1, P1REG_IGMPSNOP, 5, 1, igmp ->gdata);
	return 0;
}

int getIGMPGroupAgain(void *cdata, int len){
	struct GeneralSetting *igmp;

	ip1829drv_dbg("In getIGMPGroupAgain.\n");
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("length error!\n");
		return -EINVAL;
	}

	igmp = (struct GeneralSetting *)cdata;

	igmp ->gdata = (int)_ReadRegBits(1, P1REG_IGMPSNOP, 5, 1)?OP_FUNC_ENABLE:OP_FUNC_DISABLE;

	ip1829drv_dbg("IGMPGroupAgain %s",(igmp ->gdata==OP_FUNC_ENABLE)?"ENABLE":"DISABLE");
	return 0;
}

int setIGMPRltByCPU(void *cdata, int len){
	struct GeneralSetting *igmp;

	ip1829drv_dbg("In setIGMPRltByCPU.\n");
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("length error!\n");
		return -EINVAL;
	}

	igmp = (struct GeneralSetting *)cdata;


	switch(igmp ->gdata){
		case OP_FUNC_ENABLE:
			ip1829drv_dbg("Enable Group Again\n");
			break;
		case OP_FUNC_DISABLE:
			ip1829drv_dbg("Disable Group Again\n");
			break;
		default:
			ip1829drv_dbg("Option can't find\n");
			return -EINVAL;
	}
	ip1829drv_dbg("Set IGMPSNOP: %x\n",igmp ->gdata);
	_WriteRegBits(1, P1REG_IGMPSNOP, 13, 1, igmp ->gdata);
	return 0;
}

int getIGMPRltByCPU(void *cdata, int len){
	struct GeneralSetting *igmp;

	ip1829drv_dbg("In getIGMPRltByCPU.\n");
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("length error!\n");
		return -EINVAL;
	}

	igmp = (struct GeneralSetting *)cdata;

	igmp ->gdata = (int)_ReadRegBits(1, P1REG_IGMPSNOP, 13, 1)?OP_FUNC_ENABLE:OP_FUNC_DISABLE;

	ip1829drv_dbg("IGMPRltByCPU %s",(igmp ->gdata==OP_FUNC_ENABLE)?"ENABLE":"DISABLE");
	return 0;
}

int setIGMPPktForward(void *cdata, int len){
	u16 regdata;
	u16 rule;
	struct IgmpPacketRule *igmp;

	ip1829drv_dbg("In setIGMPPktForward.\n");
	if (sizeof(struct IgmpPacketRule) != len){
		ip1829drv_dbg("length error!\n");
		return -EINVAL;
	}
	igmp = (struct IgmpPacketRule *)cdata;

	IP2Page(1);
	switch(igmp ->packet_type){
		case OP_IGMP_PACKET_QUERY:
		case OP_IGMP_PACKET_LEAVE:
		case OP_IGMP_PACKET_UN_REG_DATA:
		case OP_IGMP_PACKET_UN_DEFINED:
			if(igmp ->rule >= OP_IGMP_RULE_GROUP_MEM){
				ip1829drv_dbg("Select Type can't support Rule 'OP_IGMP_RULE_GROUP_MEM'.\n");
				return -EINVAL;
			}
			regdata=Read_Reg(P1REG_IGMPPKTFWD_0);
			ip1829drv_dbg("Read IGMPPktForward_0 : %x\n",regdata);
			rule=0x000F<<igmp ->packet_type;
			regdata&=~rule;
			regdata|=igmp ->rule<<igmp ->packet_type;
			ip1829drv_dbg("type : %d\n",igmp ->packet_type);
			ip1829drv_dbg("rule : %x\n",igmp ->rule);
			ip1829drv_dbg("Write IGMPPktForward_0 : %x\n",regdata);
			Write_Reg(P1REG_IGMPPKTFWD_0, regdata);
			break;
		case OP_IGMP_PACKET_REPORT:
		case OP_IGMP_PACKET_GROUP_SPECIFIC_QUERY:
		case OP_IGMP_PACKET_REG_DATA:
			regdata=Read_Reg(P1REG_IGMPPKTFWD_1);
			ip1829drv_dbg("Read IGMPPktForward_1 : %x\n",regdata);
			rule=0x001F<<(igmp ->packet_type-1);
			regdata&=~rule;
			regdata|=igmp ->rule<<(igmp ->packet_type-1);
			ip1829drv_dbg("type : %d\n",igmp ->packet_type);
			ip1829drv_dbg("rule : %x\n",igmp ->rule);
			ip1829drv_dbg("Write IGMPPktForward_1 : %x\n",regdata);
			Write_Reg(P1REG_IGMPPKTFWD_1, regdata);
			break;
		default:
			ip1829drv_dbg("Option can't find.\n");
			return -EINVAL;
			break;
	}
	return 0;
}

int getIGMPPktForward(void *cdata, int len){
	u16 regdata;
	u16 rule;
	struct IgmpPacketRule *igmp;

	ip1829drv_dbg("In getIGMPPktForward.\n");
	if (sizeof(struct IgmpPacketRule) != len){
		ip1829drv_dbg("length error!\n");
		return -EINVAL;
	}
	igmp = (struct IgmpPacketRule *)cdata;

	IP2Page(1);
	switch(igmp ->packet_type){
		case OP_IGMP_PACKET_QUERY:
		case OP_IGMP_PACKET_LEAVE:
		case OP_IGMP_PACKET_UN_REG_DATA:
		case OP_IGMP_PACKET_UN_DEFINED:
			regdata=Read_Reg(P1REG_IGMPPKTFWD_0);
			ip1829drv_dbg("Read IGMPPktForward_0 : %x\n",regdata);
			rule=0x000F<<igmp ->packet_type;
			regdata&=rule;
			igmp ->rule=regdata>>igmp ->packet_type;
			ip1829drv_dbg("type : %d\n",igmp ->packet_type);
			ip1829drv_dbg("Get rule : %x\n",igmp ->rule);
			break;
		case OP_IGMP_PACKET_REPORT:
		case OP_IGMP_PACKET_GROUP_SPECIFIC_QUERY:
		case OP_IGMP_PACKET_REG_DATA:
			regdata=Read_Reg(P1REG_IGMPPKTFWD_1);
			ip1829drv_dbg("Read IGMPPktForward_1 : %x\n",regdata);
			rule=0x001F<<(igmp ->packet_type-1);
			regdata&=rule;
			igmp ->rule=regdata>>(igmp ->packet_type-1);
			ip1829drv_dbg("type : %d\n",igmp ->packet_type);
			ip1829drv_dbg("Get rule : %x\n",igmp ->rule);
			break;
		default:
			ip1829drv_dbg("Option can't find.\n");
			return -EINVAL;
			break;
	}
	return 0;
}

int setIGMPRlt(void *cdata, int len){
	u16 regdata;
	u16 map_0, map_1, value_0, value_1, i, mask;
	struct IgmpRouterListSetting *igmp;

	ip1829drv_dbg("In setIGMPRlt.\n");
	if (sizeof(struct IgmpRouterListSetting) != len){
		ip1829drv_dbg("length error!\n");
		return -EINVAL;
	}
	igmp = (struct IgmpRouterListSetting *)cdata;

	map_0=0xFFFF&igmp ->portmask;
	map_1=0xFFFF&(igmp ->portmask>>16);

	value_0=0xFFFF&igmp ->tstate;
	value_1=0xFFFF&(igmp ->tstate>>16);

	ip1829drv_dbg("map [16-01] : %x\n",map_0);
	ip1829drv_dbg("map [29-17] : %x\n",map_1);
	ip1829drv_dbg("val [16-01] : %x\n",value_0);
	ip1829drv_dbg("val [29-17] : %x\n",value_1);

	IP2Page(1);
	regdata=Read_Reg(P1REG_ROUTLIST_0);
	ip1829drv_dbg("Read [16-01] : %x\n",regdata);
	mask=0x01;
	for(i=0;i<16;i++){
		if( (map_0&mask)!=0 ){
			if( (value_0&mask)!=0 )
				regdata|=mask;
			else
				regdata&=~mask;
		}
		mask<<=1;
	}
	ip1829drv_dbg("Write new [16-01] : %x\n",regdata);
	Write_Reg(P1REG_ROUTLIST_0, regdata);

	regdata=Read_Reg(P1REG_ROUTLIST_1);
	ip1829drv_dbg("Read [29-17] : %x\n",regdata);
	mask=0x01;
	for(i=0;i<13;i++){
		if( (map_1&mask)!=0 ){
			if( (value_1&mask)!=0 )
				regdata|=mask;
			else
				regdata&=~mask;
		}
		mask<<=1;
	}
	ip1829drv_dbg("Write new [29-17] : %x\n",regdata);
	Write_Reg(P1REG_ROUTLIST_1, regdata);

	return 0;
}

int getIGMPRlt(void *cdata, int len){
	u16 regdata;
	u16 map_0, map_1, value_0, value_1, i, mask;
	struct IgmpRouterListSetting *igmp;

	ip1829drv_dbg("In getIGMPRlt.\n");
	if (sizeof(struct IgmpRouterListSetting) != len){
		ip1829drv_dbg("length error!\n");
		return -EINVAL;
	}
	igmp = (struct IgmpRouterListSetting *)cdata;

	map_0=0xFFFF&igmp ->portmask;
	map_1=0xFFFF&(igmp ->portmask>>16);
	value_0=0;
	value_1=0;

	ip1829drv_dbg("map [16-01] : %x\n",map_0);
	ip1829drv_dbg("map [29-17] : %x\n",map_1);

	IP2Page(1);
	regdata=Read_Reg(P1REG_ROUTLIST_0);
	ip1829drv_dbg("Read [16-01] : %x\n",regdata);
	mask=0x01;
	for(i=0;i<16;i++){
		if( (map_0&mask)!=0 ){
			if( (regdata&mask)!=0 )
				value_0|=mask;
			else
				value_0&=~mask;
		}
		mask<<=1;
	}

	regdata=Read_Reg(P1REG_ROUTLIST_1);
	ip1829drv_dbg("Read [29-17] : %x\n",regdata);
	mask=0x01;
	for(i=0;i<13;i++){
		if( (map_1&mask)!=0 ){
			if( (regdata&mask)!=0 )
				value_1|=mask;
			else
				value_1&=~mask;
		}
		mask<<=1;
	}

	igmp ->tstate=value_1;
	igmp ->tstate<<=16;
	igmp ->tstate|=value_0;
	ip1829drv_dbg("Return value : %lx\n",igmp ->tstate);

	return 0;
}

int setIGMPHashMethod(void *cdata, int len)
{
	int hash;

	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	hash = ((struct GeneralSetting *)cdata) ->gdata;
	if ( !(hash==OP_HASH_DIRECT || hash==OP_HASH_CRC) )
		return -EINVAL;

	_WriteRegBits(1, P1REG_IGMPPKTFWD_1, 15, 1,(hash == OP_HASH_DIRECT)?1:0);

	return 0;
}

int getIGMPHashMethod(void *cdata, int len)
{
	if (sizeof(struct GeneralSetting) != len)
		return -EINVAL;

	((struct GeneralSetting *)cdata)->gdata = (int)_ReadRegBits(1, P1REG_IGMPPKTFWD_1, 15, 1)?OP_HASH_DIRECT:OP_HASH_CRC;

	return 0;
}

int setIGMPMldRule(void *cdata, int len){
	struct GeneralSetting *igmp;

	ip1829drv_dbg("In setIGMPMldRule.\n");
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("length error!\n");
		return -EINVAL;
	}

	igmp = (struct GeneralSetting *)cdata;

	ip1829drv_dbg("gdata : %d\n",igmp ->gdata);
	_WriteRegBits(0, P0REG_IPV6RLTFWD, 8, 2, igmp ->gdata);

	return 0;
}

int getIGMPMldRule(void *cdata, int len){
	struct GeneralSetting *igmp;

	ip1829drv_dbg("In getIGMPMldRule.\n");
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("length error!\n");
		return -EINVAL;
	}

	igmp = (struct GeneralSetting *)cdata;

	igmp ->gdata= (int)_ReadRegBits(0,P0REG_IPV6RLTFWD, 8, 2);
	ip1829drv_dbg("Return gdata : %x\n",igmp ->gdata);

	return 0;
}

void makeTable(u16 *tmpbuf, u16 offset, u32 value, u8 len){
	u16 ptr, set, tmpvalue;

	ptr=offset/16;
	set=offset%16;
	ip1829drv_dbg("\n\nMakeTable:\noffset : %d\nvalue : %x\nlen : %d\n",offset, value, len);
	ip1829drv_dbg("\nbuf[%d]=%x buf[%d]=%x\n",ptr,tmpbuf[ptr],ptr+1,tmpbuf[ptr+1]);
	tmpvalue=(0xFFFF&value)<<set;
	tmpbuf[ptr]|=tmpvalue;
	ip1829drv_dbg("Result:\nbuf[%d] | %x = %x\n",ptr,tmpvalue, tmpbuf[ptr]);
	if(len>(16-set)){
		len-=(16-set);
		tmpvalue=0xFFFF&(value>>(16-set));
		tmpbuf[ptr+1]|=tmpvalue;
		ip1829drv_dbg("buf[%d] | %x = %x\n",ptr+1,tmpvalue,tmpbuf[ptr+1]);
		if(len>16){
			tmpvalue=0xFFFF&(value>>(32-set));
			tmpbuf[ptr+2]|=tmpvalue;
			ip1829drv_dbg("buf[%d] | %x = %x\n",ptr+2,tmpvalue,tmpbuf[ptr+2]);
		}
	}
}

u32 getTable(u16 *tmpbuf, u16 offset, u8 len){
	u32 value;
	u16 ptr, set, tmpvalue, i, mask;
	u16 debug;
	ip1829drv_dbg("getTable:\n");
	ip1829drv_dbg("offset = %d, len = %d",offset,len);
	debug=offset/16;
	ip1829drv_dbg("tmpbuf[%d] offset value:\n",debug);
	for(i=0;i<3;i++)
		ip1829drv_dbg("%04x ",tmpbuf[debug+i]);
	ptr=(offset+len)/16;
	set=(offset+len)%16;

	if(len>=set){
		mask=0;
		for(i=0;i<set;i++){
			mask<<=1;
			mask|=0x01;
		}
		tmpvalue=tmpbuf[ptr]&mask;
		value=tmpvalue;
		if( (len-set)>=16 ){
			value<<=16;
			value|=tmpbuf[ptr-1];
			if( (len-set-16)>0 ){
				value<<=(len-set-16);
				value|=tmpbuf[ptr-2]>>(16-(len-set-16));
			}
		}else{
			if(len!=set){
				value<<=(len-set);
				value|=tmpbuf[ptr-1]>>(16-(len-set));
			}
		}
	}else{
		mask=0;
		for(i=0;i<set;i++){
			mask<<=1;
			mask|=0x01;
		}
		tmpvalue=tmpbuf[ptr]&mask;
		value=tmpvalue>>(set-len);
	}
	ip1829drv_dbg("return %x\n",value);
	return value;
}

/*	ipv:	4:	IPv4
 *			6:	IPv6
 *	addr:	IP	address
 *	method:	1:	Direct
 *			0:	CRC
 * */
u8 tb_calc_index(u8 ipv, void *addr, u8 method)
{
	u8 index = 0;
	u8 *u8_addr = (u8*)addr;

	// Direct
	if(method)
	{
		if(ipv == 4)
		{
			index = u8_addr[3];
		}
		else if(ipv == 6)
		{
			index = u8_addr[15];
		}
	}
	// CRC
	else
	{
		int i;
		bool crc[8], d[32];
		bool c[8] = {1,1,1,1,1,1,1,1};

		// init data
		if(ipv == 4)
		{
			for(i=0; i<32; i++)
				d[i] = (bool)((u8_addr[i/8] >> (7-(i%8))) & 0x1);
		}
		else if(ipv == 6)
		{
			for(i=0; i<32; i++)
				d[i] = (bool)((u8_addr[12+(i/8)] >> (7-(i%8))) & 0x1);
		}

		// calculate crc
		crc[0] = d[31] ^ d[30] ^ d[28] ^ d[23] ^ d[21] ^ d[19] ^ d[18] ^ d[16] ^ d[14] ^ d[12] ^ d[8] ^ d[7] ^ d[6] ^ d[0] ^ c[4] ^ c[6] ^ c[7];

		crc[1] = d[30] ^ d[29] ^ d[28] ^ d[24] ^ d[23] ^ d[22] ^ d[21] ^ d[20] ^ d[18] ^ d[17] ^ d[16] ^ d[15] ^ d[14] ^ d[13] ^ d[12] ^ d[9] ^ d[6] ^ d[1] ^ d[0] ^ c[0] ^ c[4] ^ c[5] ^ c[6];

		crc[2] = d[29] ^ d[28] ^ d[25] ^ d[24] ^ d[22] ^ d[17] ^ d[15] ^ d[13] ^ d[12] ^ d[10] ^ d[8] ^ d[6]^ d[2] ^ d[1] ^ d[0] ^ c[0] ^ c[1] ^ c[4] ^ c[5];

		crc[3] = d[30] ^ d[29] ^ d[26] ^ d[25] ^ d[23] ^ d[18] ^ d[16] ^ d[14] ^ d[13] ^ d[11] ^ d[9] ^ d[7] ^ d[3] ^ d[2] ^ d[1] ^ c[1] ^ c[2] ^ c[5] ^ c[6];

		crc[4] = d[31] ^ d[30] ^ d[27] ^ d[26] ^ d[24] ^ d[19] ^ d[17] ^ d[15] ^ d[14] ^ d[12] ^ d[10] ^ d[8] ^ d[4] ^ d[3] ^ d[2] ^ c[0] ^ c[2] ^ c[3] ^ c[6] ^ c[7];

		crc[5] = d[31] ^ d[28] ^ d[27] ^ d[25] ^ d[20] ^ d[18] ^ d[16] ^ d[15] ^ d[13] ^ d[11] ^ d[9] ^ d[5] ^ d[4] ^ d[3] ^ c[1] ^ c[3] ^ c[4] ^ c[7];

		crc[6] = d[29] ^ d[28] ^ d[26] ^ d[21] ^ d[19] ^ d[17] ^ d[16] ^ d[14] ^ d[12] ^ d[10] ^ d[6] ^ d[5] ^ d[4] ^ c[2] ^ c[4] ^ c[5];

		crc[7] = d[30] ^ d[29] ^ d[27] ^ d[22] ^ d[20] ^ d[18] ^ d[17] ^ d[15] ^ d[13] ^ d[11] ^ d[7] ^ d[6] ^ d[5] ^ c[3] ^ c[5] ^ c[6];

		// get index
		for(i=0; i<8; i++)
			index |= ((u8)crc[i])<<i;
	}

	ip1829drv_dbg("tb_calc_index() index = %02u\n", index);
	return index;
}

int setIGMPMctTable(void *cdata, int len){
	u16 regdata[11]={0}, i, tmpvalue;
	//u32 mask;
	struct MtRuleSetting *igmp;

	ip1829drv_dbg("In setIGMPMctTable.\n");
	if (sizeof(struct MtRuleSetting) != len)
	{
		ip1829drv_dbg("length error!\n");
		return -EINVAL;
	}

	igmp = (struct MtRuleSetting *)cdata;

	memset(regdata, 0, 11);
	ip1829drv_dbg("driver:index: %x\n",igmp ->index);
	ip1829drv_dbg("driver:group: %x : %x : %x : %x\n",igmp->mt_data.group[0], igmp->mt_data.group[1], igmp->mt_data.group[2], igmp->mt_data.group[3]);
	ip1829drv_dbg("driver:Set FID: %x\n",igmp ->mt_data.fid);
	ip1829drv_dbg("driver:Set age port:: %lx\n",igmp ->mt_data.port_mem);
	ip1829drv_dbg("driver:Set SIP: %x\n",igmp ->mt_data.slt_index);
	ip1829drv_dbg("driver:Set filter: %lx\n",igmp ->mt_data.port_filter_mode);
	ip1829drv_dbg("driver:Set flag: %x\n",igmp ->mt_data.flag);

	//set Group
	regdata[0] = (igmp->mt_data.group[2] << 8) | igmp->mt_data.group[3];
	regdata[1] = (igmp->mt_data.group[0] << 8) | igmp->mt_data.group[1];
	//set FID
	makeTable(regdata, 32, igmp ->mt_data.fid, 4);
	//set Aging Member

	for(i=0; i<28; i++)
	{
		if(igmp->mt_data.port_mem & BIT(i))
			makeTable(regdata, 36+3*i, 0x7, 3);
	}
	//set SIP
	makeTable(regdata, 123, igmp ->mt_data.slt_index, 6);
	//set Filter mode
	makeTable(regdata, 129, igmp ->mt_data.port_filter_mode, 29);
	//set Pri, snif, ver and CPU
	makeTable(regdata, 158, igmp ->mt_data.flag, 7);

	IP2Page(1);
	ip1829drv_dbg("driver:Write Mct Table Data:\n");
	for(i=0;i<11;i++){
		ip1829drv_dbg("%04x ",regdata[i]);
		Write_Reg(P1REG_MEM_TABLE_0+i, regdata[i]);
	}
	ip1829drv_dbg("\n");

	tmpvalue=igmp ->index;
	tmpvalue|=0x01<<8;
	tmpvalue|=0xC000;
	ip1829drv_dbg("\ndirver:Start Write [%04x]",tmpvalue);
	Write_Reg(P1REG_MEM_COMMAND, tmpvalue);

	tmpvalue=Read_Reg(P1REG_MEM_COMMAND);
	while( (tmpvalue&0x8000)!=0 )
		tmpvalue=Read_Reg(P1REG_MEM_COMMAND);

	return 0;
}

int getIGMPMctTable(void *cdata, int len){
	u16 regdata[11]={0}, i, tmpvalue;
	u32 mask;
	struct MtRuleSetting *igmp;

	ip1829drv_dbg("In getIGMPMctTable.\n");
	if (sizeof(struct MtRuleSetting) != len)
	{
		ip1829drv_dbg("length error!\n");
		return -EINVAL;
	}

	igmp = (struct MtRuleSetting *)cdata;

	tmpvalue=igmp ->index;
	tmpvalue|=0x01<<8;
	tmpvalue|=0x8000;
	IP2Page(1);
	Write_Reg(P1REG_MEM_COMMAND, tmpvalue);
	ip1829drv_dbg("driver:start read: %x\n",tmpvalue);

	tmpvalue=Read_Reg(P1REG_MEM_COMMAND);
	while( (tmpvalue&0x8000)!=0 )
		tmpvalue=Read_Reg(P1REG_MEM_COMMAND);

	ip1829drv_dbg("driver:index: %x\n",igmp ->index);
	ip1829drv_dbg("driver:Read Table Data:\n");
	for(i=0;i<11;i++){
		regdata[i]=Read_Reg(P1REG_MEM_TABLE_0+i);
		ip1829drv_dbg("%04x ",regdata[i]);
	}
	ip1829drv_dbg("\nRead Group:\n");
	for(i=0;i<4;i++){
		igmp ->mt_data.group[i]=getTable(regdata, (0+(8*i)), 8);
		ip1829drv_dbg("%x ",igmp ->mt_data.group[i]);
	}

	ip1829drv_dbg("\nRead FID:\n");
	igmp ->mt_data.fid=getTable(regdata, 32, 4);
	ip1829drv_dbg("%x\n",igmp ->mt_data.fid);

	ip1829drv_dbg("Read Aging:\n");
	mask=0;
	for(i=0;i<29;i++){
		if( getTable(regdata, (36+(3*i)), 3)!=0 )
			mask|=0x01<<i;
	}
	ip1829drv_dbg("%x\n",mask);
	igmp ->mt_data.port_mem=mask;

	ip1829drv_dbg("Read SLT index:\n");
	igmp ->mt_data.slt_index=getTable(regdata, 123, 6);
	ip1829drv_dbg("%x\n",igmp ->mt_data.slt_index);

	ip1829drv_dbg("Read Filter mode:\n");
	igmp ->mt_data.port_filter_mode=getTable(regdata, 129, 29);
	ip1829drv_dbg("%lx\n",igmp ->mt_data.port_filter_mode);

	ip1829drv_dbg("Read flag:\n");
	igmp ->mt_data.flag=getTable(regdata, 158, 7);
	ip1829drv_dbg("%x\n",igmp ->mt_data.flag);

	return 0;
}

int setIGMPSltTable(void *cdata, int len){
	u16 regdata[40]={0}, i, j, tmpvalue;
	struct SltRuleSetting *igmp;

	ip1829drv_dbg("In setIGMPSltTable.\n");
	if (sizeof(struct SltRuleSetting) != len)
	{
		ip1829drv_dbg("length error!\n");
		return -EINVAL;
	}

	igmp = (struct SltRuleSetting *)cdata;
	if(igmp ->slt_data.type==OP_IGMP_SLT_IPV4){
		//set IP
		for(i=0;i<10;i++){
			regdata[i*2] = (igmp->slt_data.data.ipv4.ip[i][2] << 8) | igmp->slt_data.data.ipv4.ip[i][3];
			regdata[i*2+1] = (igmp->slt_data.data.ipv4.ip[i][0] << 8) | igmp->slt_data.data.ipv4.ip[i][1];
			//for(j=0;j<4;j++)
			//	makeTable(regdata, ((32*i)+(8*j)), igmp ->slt_data.data.ipv4.ip[i][j], 8);
		}
		//set User Define
		for(i=0;i<10;i++)
			makeTable(regdata, (320+(29*i)), igmp ->slt_data.data.ipv4.used_port[i], 29);
	}else{
		//set IP
		for(i=0;i<4;i++){
			for(j=0;j<8;j++)
				makeTable(regdata, ((128*i)+(16*j)), igmp ->slt_data.data.ipv6.ip[i][7-j], 16);
		}
		//set User Define
		for(i=0;i<4;i++)
			makeTable(regdata, (512+(29*i)), igmp ->slt_data.data.ipv6.used_port[i], 29);
	}

	ip1829drv_dbg("SLT Table Data:");
	IP2Page(1);
	for(i=0;i<40;i++){
		if((i%16)==0)
			ip1829drv_dbg("\n");
		ip1829drv_dbg("%04x ",regdata[i]);
		Write_Reg(P1REG_MEM_TABLE_0+i, regdata[i]);
	}
	tmpvalue=igmp ->index;
	tmpvalue|=0xC000;
	ip1829drv_dbg("\ndirver:Start Write [%04x]",tmpvalue);
	Write_Reg(P1REG_MEM_COMMAND, tmpvalue);

	return 0;
}

int getIGMPSltTable(void *cdata, int len){
	u16 regdata[39]={0}, i, j, tmpvalue;
	struct SltRuleSetting *igmp;

	ip1829drv_dbg("In getIGMPSltTable.\n");
	if (sizeof(struct SltRuleSetting) != len)
	{
		ip1829drv_dbg("length error!\n");
		return -EINVAL;
	}

	igmp = (struct SltRuleSetting *)cdata;

	tmpvalue=igmp ->index;
	tmpvalue|=0x8000;
	IP2Page(1);
	Write_Reg(P1REG_MEM_COMMAND, tmpvalue);

	tmpvalue=Read_Reg(P1REG_MEM_COMMAND);
	while( (tmpvalue&0x8000)!=0 )
		tmpvalue=Read_Reg(P1REG_MEM_COMMAND);

	ip1829drv_dbg("Read SLT Table:");
	for(i=0;i<39;i++){
		regdata[i]=Read_Reg(P1REG_MEM_TABLE_0+i);
		if(i%16==0)
			ip1829drv_dbg("\n");
		ip1829drv_dbg("%04x ",regdata[i]);
	}

	if(igmp ->slt_data.type==OP_IGMP_SLT_IPV4){
		ip1829drv_dbg("Read IPv4:\n");
		for(i=0;i<10;i++){
			for(j=0;j<4;j++)
				igmp ->slt_data.data.ipv4.ip[i][j]=getTable(regdata, ((32*i)+(8*j)), 8 );
		}
		ip1829drv_dbg("Read Used Define:\n");
		for(i=0;i<10;i++)
			igmp ->slt_data.data.ipv4.used_port[i]=getTable(regdata, (320+(29*i)), 29);
	}else{
		ip1829drv_dbg("Read IPv6:\n");
		for(i=0;i<4;i++){
			for(j=0;j<8;j++)
				igmp ->slt_data.data.ipv6.ip[i][j]=getTable(regdata, ((128*i)+(16*j)), 16);
		}
		ip1829drv_dbg("Read Used Define:\n");
		for(i=0;i<4;i++)
			igmp ->slt_data.data.ipv6.used_port[i]=getTable(regdata, (512+(29*i)), 29);
	}

	return 0;
}

//------------ IGMP functions:common end-------------------------

//------------ IMP functions:common -----------------------------
int setImpMode(void *cdata, int len)
{
	u16 mode;

	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	mode = ((struct GeneralSetting *)cdata) ->gdata;
	if (mode != OP_IMP_DROP_IP_MISMATCH && mode !=  OP_IMP_DROP_IP_MATCH)
	{
		ip1829drv_dbg("Error: mode=%X\n", mode);
		return -EINVAL;
	}

	ip1829drv_dbg("mode=0x%08x\n", mode);

	_WriteRegBits(2, P2REG_IMPCFG1, 13, 1, mode);
	FUNC_MSG_OUT;
	return 0;
}

int getImpMode(void *cdata, int len)
{
	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	((struct GeneralSetting *)cdata) ->gdata = (int)_ReadRegBits(2,P2REG_IMPCFG1, 13, 1);

	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	FUNC_MSG_OUT;
	return 0;
}

int setImpPassNullIP(void *cdata, int len)
{
	u16 mode;

	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	mode = ((struct GeneralSetting *)cdata) ->gdata;
	if (mode != OP_FUNC_ENABLE && mode !=  OP_FUNC_DISABLE)
	{
		ip1829drv_dbg("Error: mode=%X\n", mode);
		return -EINVAL;
	}

	ip1829drv_dbg("mode=0x%08x\n", mode);

	_WriteRegBits(2, P2REG_IMPCFG1, 14, 1, mode);
	FUNC_MSG_OUT;
	return 0;
}

int getImpPassNullIP(void *cdata, int len)
{
	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	((struct GeneralSetting *)cdata) ->gdata = (int)_ReadRegBits(2,P2REG_IMPCFG1, 14, 1);

	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	FUNC_MSG_OUT;
	return 0;
}

int setImpHash(void *cdata, int len)
{
	u16 mode;

	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	mode = ((struct GeneralSetting *)cdata) ->gdata;
	if (mode != OP_HASH_DIRECT && mode !=  OP_HASH_CRC)
	{
		ip1829drv_dbg("Error: mode=%X\n", mode);
		return -EINVAL;
	}

	ip1829drv_dbg("mode=0x%08x\n", mode);

	_WriteRegBits(2, P2REG_IMPCFG1, 15, 1, (mode==OP_HASH_DIRECT)?1:0);
	FUNC_MSG_OUT;
	return 0;
}

int getImpHash(void *cdata, int len)
{
	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	((struct GeneralSetting *)cdata) ->gdata = _ReadRegBits(2, P2REG_IMPCFG1, 15, 1)?OP_HASH_DIRECT:OP_HASH_CRC;
	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	FUNC_MSG_OUT;
	return 0;
}

int setImpPort(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _setPortmapMask(cdata, len, 2, P2REG_IMPCFG0);
	FUNC_MSG_IN;
	return ret;
}

int getImpPort(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _getPortmapMask(cdata, len, 2, P2REG_IMPCFG0);
	FUNC_MSG_OUT;
	return ret;
}

static unsigned char IMP_Hash(u8 ip[16], bool ip_type)
{
	u8 index=0, loop_num, i, j;
	bool crc[7];
	bool c[7];
	bool d[32];

	FUNC_MSG_IN;
	loop_num =(ip_type==OP_IMP_IPTYPE_4)?1:4;

	/* initialize crc value */
	for(i=0; i<7; i++)
		crc[i] = 1;

	for(j=0; j<loop_num; j++)
	{
		for(i=0; i<32; i++)
			d[i] = (ip[j*4+(31-i)/8]>>(i%8))&0x1;
		for(i=0; i<7; i++)
			c[i] = crc[i];

		crc[0]=c[0]^c[1]^c[2]^c[3]^c[4]^\
			   d[0]^d[1]^d[2]^d[3]^d[4]^d[5]^d[12]^d[13]^d[14]^d[15]^d[16]^d[17]^d[24]^d[25]^d[26]^d[27]^d[28]^d[29];
		crc[1]=c[5]^\
			   d[0]^d[6]^d[12]^d[18]^d[24]^d[30];
		crc[2]=c[0]^c[6]^\
			   d[1]^d[7]^d[13]^d[19]^d[25]^d[31];
		crc[3]=c[1]^\
			   d[2]^d[8]^d[14]^d[20]^d[26];
		crc[4]=c[2]^\
			   d[3]^d[9]^d[15]^d[21]^d[27];
		crc[5]=c[3]^\
			   d[4]^d[10]^d[16]^d[22]^d[28];
		crc[6]=c[0]^c[1]^c[2]^c[3]^\
			   d[0]^d[1]^d[2]^d[3]^d[4]^d[11]^d[12]^d[13]^d[14]^d[15]^d[16]^d[23]^d[24]^d[25]^d[26]^d[27]^d[28];
	}

	for(i=0; i<7; i++)
		index |= (crc[i]<<i);

	ip1829drv_dbg("imp_hash=%d\n", index);
	FUNC_MSG_OUT;
	return index;
}

int setImpEntry(void *cdata, int len)
{
	int i;
	u16 u16dat;
	u8 index;
	struct IP1829IMPSetting *imp;

	FUNC_MSG_IN;

	/* check cdata length */
	if (sizeof(struct IP1829IMPSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	imp = (struct IP1829IMPSetting*)cdata;

	/* check action */
	if(imp ->action != OP_ENTRY_CREATE && imp ->action != OP_ENTRY_CONFIG && imp ->action != OP_ENTRY_DELETE)
	{
		ip1829drv_dbg("action error:%d\n", imp ->action);
		return -EINVAL;
	}

	/* check source port range */
	if(imp ->entry.srcport>SWITCH_MAX_IPORT_CNT)
	{
		ip1829drv_dbg("srcport error:%d\n", imp ->entry.srcport);
		return -EINVAL;
	}

	/* check priority range */
	if(imp ->entry.priority>15)
	{
		ip1829drv_dbg("priority error:%d\n", imp ->entry.priority);
		return -EINVAL;
	}

	/* initialize retval value */
	imp ->retval = 0;
	/* calculate hash value */
	index = IMP_Hash(imp ->entry.ip, imp ->entry.flag.ip_type);

	ip1829drv_dbg("\n\nindex=%d\n", index);
	if(imp ->entry.flag.ip_type==OP_IMP_IPTYPE_4)
	{
		ip1829drv_dbg("IPv4=");
		for(i=0; i<4; i++)
			ip1829drv_dbg("[%02X]", imp ->entry.ip[i]);
		ip1829drv_dbg("\n");
	}
	else
	{
		ip1829drv_dbg("IPv6=");
		for(i=0; i<16; i++)
			ip1829drv_dbg("[%02X]", imp ->entry.ip[i]);
		ip1829drv_dbg("\n");
	}
	ip1829drv_dbg("MAC=");
	for(i=0; i<6; i++)
		ip1829drv_dbg("[%02X]", imp ->entry.mac[i]);
	ip1829drv_dbg("\n");

	ip1829drv_dbg("srcport=%d\n", imp ->entry.srcport);
	ip1829drv_dbg("priority=%d\n", imp ->entry.priority);
	ip1829drv_dbg("valid=%d\n", imp ->entry.flag.valid);
	ip1829drv_dbg("ip_type=%d\n", imp ->entry.flag.ip_type);
	ip1829drv_dbg("check_ip=%d\n", imp ->entry.flag.check_ip);
	ip1829drv_dbg("check_mac=%d\n", imp ->entry.flag.check_mac);
	ip1829drv_dbg("check_port=%d\n", imp ->entry.flag.check_port);
	ip1829drv_dbg("filter=%d\n", imp ->entry.flag.filter);
	ip1829drv_dbg("sniff2=%d\n", imp ->entry.flag.sniff2);

	/* set to register page 1 */
	IP2Page(1);

	/* read entry from IP table to check this index is valid or not */
	u16dat = index;		//set index
	u16dat |= 0x8200;	//IP-table reading command
	Write_Reg(P1REG_MEM_COMMAND, u16dat);

	/* wait to command ack */
	while((Read_Reg(P1REG_MEM_COMMAND)&0x8000));

	u16dat = Read_Reg(P1REG_MEM_TABLE_0+11);

	if(imp ->action==OP_ENTRY_CREATE)
	{
		/* check valid bit of target entry */
		if(((u16dat>>11)&0x01)==OP_FUNC_ENABLE)
		{
			ip1829drv_dbg("target entry is not empty\n");
			imp ->retval = OP_ENTRY_EXISTS;
			return 0;
		}
	}
	else
	{
		/* check valid bit of target entry */
		if(((u16dat>>11)&0x01)==OP_FUNC_DISABLE)
		{
			ip1829drv_dbg("target entry is not fount.\n");
			imp ->retval = OP_ENTRY_NOT_FOUND;
			return 0;
		}

		/*check IP type is match or not */
		if(((u16dat>>8)&0x01)!= imp ->entry.flag.ip_type)
		{
			ip1829drv_dbg("ip_type is not match\n");
			imp ->retval = OP_ENTRY_NOT_MATCH;
			return 0;
		}

		/* if target entry is valid, check ip field matched or not */
		if(imp ->entry.flag.ip_type==OP_IMP_IPTYPE_4)
		{
			/* read ipv4 to check match */
			for(i=0; i<2; i++ )
			{
				u16dat = imp ->entry.ip[0+(1-i)*2]<<8 | imp ->entry.ip[1+(1-i)*2];
				if(Read_Reg(P1REG_MEM_TABLE_0+i) != u16dat)
				{
					ip1829drv_dbg("IPv4 value is not match\n");
					imp ->retval = OP_ENTRY_NOT_MATCH;
					return 0;
				}
			}
		}
		else
		{
			/* read ipv6 to check match */
			for(i=0; i<8; i++ )
			{
				u16dat = imp ->entry.ip[0+(7-i)*2]<<8 | imp ->entry.ip[1+(7-i)*2];
				if(Read_Reg(P1REG_MEM_TABLE_0+i) != u16dat)
				{
					ip1829drv_dbg("IPv6 value is not match\n");
					imp ->retval = OP_ENTRY_NOT_MATCH;
					return 0;
				}
			}
		}
	}

	/* write entry to IP table */
	if(imp ->action!=OP_ENTRY_DELETE)
	{
		/* set IP value & priority */
		if(imp ->entry.flag.ip_type==OP_IMP_IPTYPE_4)
		{
			/* set IPv4 */
			for(i=0; i<2; i++ )
			{
				u16dat = imp ->entry.ip[0+(1-i)*2]<<8 | imp ->entry.ip[1+(1-i)*2];
				Write_Reg(P1REG_MEM_TABLE_0+i, u16dat );
			}
			/* set priority */
			u16dat = imp ->entry.priority;
			Write_Reg(P1REG_MEM_TABLE_0+2, u16dat );
		}
		else
		{
			/* set IPv6 */
			for(i=0; i<8; i++ )
			{
				u16dat = imp ->entry.ip[0+(7-i)*2]<<8 | imp ->entry.ip[1+(7-i)*2];
				Write_Reg(P1REG_MEM_TABLE_0+i, u16dat );
			}
		}

		/* set MAC */
		for(i=0; i<3; i++ )
		{
			u16dat = imp ->entry.mac[0+(2-i)*2]<<8 | imp ->entry.mac[1+(2-i)*2];
			Write_Reg(P1REG_MEM_TABLE_0+8+i, u16dat );
		}

		/* set optional setting(reg 3A) */
		u16dat = OP_FUNC_ENABLE<<11;
		u16dat |= (imp ->entry.flag.sniff2)<<10;
		u16dat |= ((imp ->entry.flag.filter==OP_FUNC_ENABLE)?0x0:0x1)<<9;
		u16dat |= (imp ->entry.flag.ip_type)<<8;
		if(imp->entry.flag.check_port)
			u16dat |= (BIT(7) | (imp->entry.srcport-1));
		u16dat |= (imp ->entry.flag.check_mac)<<6;
		u16dat |= (imp ->entry.flag.check_ip)<<5;
		Write_Reg(P1REG_MEM_TABLE_0+11, u16dat);
	}
	else
	{
		/* set optional setting(reg 3A) */
		u16dat = OP_FUNC_DISABLE<<11;
		Write_Reg(P1REG_MEM_TABLE_0+11, u16dat);
	}

	/* set access command register */
	u16dat = index;		//set index
	u16dat |= 0xC200;	//IP-table reading command
	Write_Reg(P1REG_MEM_COMMAND, u16dat);

	/* wait to command ack */
	while((Read_Reg(P1REG_MEM_COMMAND)&0x8000));

	FUNC_MSG_OUT;
	return 0;
}

int getImpEntry(void *cdata, int len)
{
	int i;
	bool f_notMatch = 0;
	u16 u16dat;
	u8 index;
	struct IP1829IMPSetting *imp;

	FUNC_MSG_IN;

	/* check cdata length */
	if (sizeof(struct IP1829IMPSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	imp = (struct IP1829IMPSetting*)cdata;

	/* check index range */
	if(imp ->index > 128)
	{
		ip1829drv_dbg("Error: index=%d\n", imp ->index);
		return -EINVAL;
	}

	/* set to register page 1 */
	IP2Page(1);

	/* check action */
	if(imp ->action == OP_ENTRY_GET_BY_INDEX)
	{
		index = imp ->index;
	}
	else if(imp ->action == OP_ENTRY_GET_BY_INFO)
	{
		/* calculate hash value */
		index = IMP_Hash(imp ->entry.ip, imp ->entry.flag.ip_type);
	}
	else
	{
		ip1829drv_dbg("action error:%d\n", imp ->action);
		return -EINVAL;
	}
	ip1829drv_dbg("index=%d\n", index);

	u16dat = index;		//set index
	u16dat |= 0x8200;	//IP-table reading command
	Write_Reg(P1REG_MEM_COMMAND, u16dat);

	/* wait to command ack */
	while((Read_Reg(P1REG_MEM_COMMAND)&0x8000));

	/* Read options value*/
	u16dat = Read_Reg(P1REG_MEM_TABLE_0+11);
	if(((u16dat>>11)&0x01) == OP_FUNC_DISABLE)
	{
		imp ->retval = OP_ENTRY_NOT_FOUND;
		return 0;
	}

	if(imp ->entry.flag.ip_type != ((u16dat>>8)&0x01))
	{
		ip1829drv_dbg("ip type is not match!\n");
		f_notMatch = 1;
	}
	else
	{
		imp ->entry.flag.valid = (u16dat>>11)&0x01;
		imp ->entry.flag.sniff2 = (u16dat>>10)&0x01;
		imp ->entry.flag.filter = (((u16dat>>9)&0x01)==0x0)?OP_FUNC_ENABLE:OP_FUNC_DISABLE;
		imp ->entry.flag.check_port = (u16dat>>7)&0x01;
		imp ->entry.flag.check_mac = (u16dat>>6)&0x01;
		imp ->entry.flag.check_ip = (u16dat>>5)&0x01;
		imp ->entry.srcport = (u16dat)&0x001f;
		ip1829drv_dbg("ip valid  %d index=%d\n",imp ->entry.flag.valid,index);
	}

	/* get IP value & priority */
	if(imp ->entry.flag.ip_type==OP_IMP_IPTYPE_4)
	{
		/* set IPv4 */
		for(i=0; i<2; i++ )
		{
			u16dat = Read_Reg(P1REG_MEM_TABLE_0+i);
			if(imp ->action == OP_ENTRY_GET_BY_INFO)
			{
				if(	(imp ->entry.ip[0+(1-i)*2] != ((u16dat>>8)&0xff))
						||	(imp ->entry.ip[1+(1-i)*2] != (u16dat&0xff)) )
				{
					ip1829drv_dbg("IPv4 is not match!\n");
					f_notMatch = 1;
				}
			}
			else
			{
				imp ->entry.ip[0+(1-i)*2] = (u16dat>>8)&0xff;
				imp ->entry.ip[1+(1-i)*2] = u16dat&0xff;
			}
		}
		/* set priority */
		u16dat = Read_Reg(P1REG_MEM_TABLE_0+2);
		imp ->entry.priority = u16dat;
	}
	else
	{
		/* set IPv6 */
		for(i=0; i<8; i++ )
		{
			u16dat = Read_Reg(P1REG_MEM_TABLE_0+i);
			if(imp ->action == OP_ENTRY_GET_BY_INFO)
			{
				if(	(imp ->entry.ip[0+(7-i)*2] != ((u16dat>>8)&0xff))
						||	(imp ->entry.ip[1+(7-i)*2] != (u16dat&0xff)) )
				{
					ip1829drv_dbg("IPv6 is not match!\n");
					f_notMatch = 1;

				}
			}
			else
			{
				imp ->entry.ip[0+(7-i)*2] = (u16dat>>8)&0xff;
				imp ->entry.ip[1+(7-i)*2] = u16dat&0xff;
			}
		}
	}

	/* get MAC */
	for(i=0; i<3; i++ )
	{
		u16dat = Read_Reg(P1REG_MEM_TABLE_0+8+i);
		imp ->entry.mac[0+(2-i)*2] = (u16dat>>8)&0xff;
		imp ->entry.mac[1+(2-i)*2] = u16dat&0xff;
	}

	/* initialize retval value */
	imp ->retval = 0;

	if(imp ->entry.flag.valid == OP_FUNC_DISABLE)
	{
		ip1829drv_dbg("Error: target entry is invalid\n");
		imp ->retval = OP_ENTRY_NOT_FOUND;
		return 0;
	}
	if(imp ->entry.flag.valid == OP_FUNC_ENABLE && f_notMatch)
	{
		ip1829drv_dbg("Error: IP value is not match\n");
		imp ->retval = OP_ENTRY_NOT_MATCH;
		return 0;
	}
	if(imp ->entry.flag.valid == OP_FUNC_ENABLE)
	{
		imp ->retval = OP_ENTRY_EXISTS;
	}

	ip1829drv_dbg("index=%d\n", index);
	if(imp ->entry.flag.ip_type==OP_IMP_IPTYPE_4)
	{
		ip1829drv_dbg("IPv4=\n");
		for(i=0; i<4; i++)
			ip1829drv_dbg("[%02X]\n", imp ->entry.ip[i]);
	}
	else
	{
		ip1829drv_dbg("IPv6=\n");
		for(i=0; i<16; i++)
			ip1829drv_dbg("[%02X]\n", imp ->entry.ip[i]);
	}
	ip1829drv_dbg("MAC=\n");
	for(i=0; i<6; i++)
		ip1829drv_dbg("[%02X]\n", imp ->entry.mac[i]);
	ip1829drv_dbg("srcport=%d\n", imp ->entry.srcport);
	ip1829drv_dbg("priority=%d\n", imp ->entry.priority);
	ip1829drv_dbg("valid=%d\n", imp ->entry.flag.valid);
	ip1829drv_dbg("check_ip=%d\n", imp ->entry.flag.check_ip);
	ip1829drv_dbg("check_mac=%d\n", imp ->entry.flag.check_mac);
	ip1829drv_dbg("check_port=%d\n", imp ->entry.flag.check_port);
	ip1829drv_dbg("filter=%d\n", imp ->entry.flag.filter);
	ip1829drv_dbg("snif2=%d\n", imp ->entry.flag.sniff2);
	FUNC_MSG_OUT;
	return 0;
}

//------------ IMP functions:common end--------------------------
//------------ CoS functions:common ----------------------------
int setCosTcpUdpUserDefine(void *cdata, int len)
{
	unsigned int ptcl;
	int act;

	FUNC_MSG_IN;
	if (sizeof(struct CapActSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	ptcl = ((struct CapActSetting *)cdata) ->protocol;
	act = ((struct CapActSetting *)cdata) ->act;

	if( ptcl >= OP_TCPUDP_USER_TOTALNUM)
	{
		ip1829drv_dbg("Error: protocol=%X\n", ptcl);
		return -EINVAL;
	}

	if( act < 0x0 || act > 0xFFFF )
	{
		ip1829drv_dbg("Error: act=%X\n", act);
		return -EINVAL;
	}

	ip1829drv_dbg("cdata ->ptcl=%X\n", ptcl);
	ip1829drv_dbg("cdata ->act=%X\n", act);

	IP2Page(0);
	Write_Reg(P0REG_TCPUDPUSERDEF + ptcl, (u16)act);

	FUNC_MSG_OUT;
	return 0;
}

int getCosTcpUdpUserDefine(void *cdata, int len)
{
	unsigned int ptcl;
	int act;

	FUNC_MSG_IN;
	if (sizeof(struct CapActSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	ptcl = ((struct CapActSetting *)cdata) ->protocol;

	if( ptcl >= OP_TCPUDP_USER_TOTALNUM)
	{
		ip1829drv_dbg("Error: protocol=%X\n", ptcl);
		return -EINVAL;
	}

	IP2Page(0);
	act = Read_Reg(P0REG_TCPUDPUSERDEF + ptcl);
	((struct CapActSetting *)cdata) ->act = act;

	ip1829drv_dbg("cdata ->protocol=%X\n", ((struct CapActSetting *)cdata) ->protocol);
	ip1829drv_dbg("cdata ->act=%X\n", ((struct CapActSetting *)cdata) ->act);
	FUNC_MSG_OUT;
	return 0;
}

int setCosTcpUdpQueue(void *cdata, int len)
{
	unsigned int ptcl;
	int act;

	FUNC_MSG_IN;
	if (sizeof(struct CapActSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	ptcl = ((struct CapActSetting *)cdata) ->protocol;
	act = ((struct CapActSetting *)cdata) ->act;

	if( ptcl >= OP_TCPUDP_PTCL_TOTALNUM)
	{
		ip1829drv_dbg("Error: protocol=%X\n", ptcl);
		return -EINVAL;
	}

	if( act < OP_TCPUDP_ACT_Q0 || act > OP_TCPUDP_ACT_ALL_PORT || act == 0x8)
	{
		ip1829drv_dbg("Error: act=%X\n", act);
		return -EINVAL;
	}

	ip1829drv_dbg("cdata ->ptcl=%X\n", ptcl);
	ip1829drv_dbg("cdata ->act=%X\n", act);

	_WriteRegBits(0, P0REG_TCPUDPPRICFG + ptcl/4, (ptcl%4)*4, 4, act);

	FUNC_MSG_OUT;
	return 0;
}

int getCosTcpUdpQueue(void *cdata, int len)
{
	unsigned int ptcl;
	int act;

	FUNC_MSG_IN;
	if (sizeof(struct CapActSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	ptcl = ((struct CapActSetting *)cdata) ->protocol;

	if( ptcl >= OP_TCPUDP_PTCL_TOTALNUM)
	{
		ip1829drv_dbg("Error: protocol=%X\n", ptcl);
		return -EINVAL;
	}

	act = _ReadRegBits(0, P0REG_TCPUDPPRICFG + (ptcl/4), (ptcl%4)*4, 4);
	((struct CapActSetting *)cdata) ->act = act;

	ip1829drv_dbg("cdata ->protocol=%X\n", ((struct CapActSetting *)cdata) ->protocol);
	ip1829drv_dbg("cdata ->act=%X\n", ((struct CapActSetting *)cdata) ->act);
	FUNC_MSG_OUT;
	return 0;
}

int setCosTcpUdpEnable(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _setPortmap(cdata, len, 0, P0REG_TCPUDFUNCEN);
	FUNC_MSG_OUT;
	return ret;
}

int getCosTcpUdpEnable(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _getPortmapMask(cdata, len, 0, P0REG_TCPUDFUNCEN);
	FUNC_MSG_OUT;
	return ret;
}

int setCosTcpEnable(void *cdata, int len)
{
	unsigned int ptcl;
	int act;

	FUNC_MSG_IN;
	if (sizeof(struct CapActSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	ptcl = ((struct CapActSetting *)cdata) ->protocol;
	act = ((struct CapActSetting *)cdata) ->act;

	if( ptcl >= OP_TCPUDP_PTCL_TOTALNUM)
	{
		ip1829drv_dbg("Error: protocol=%X\n", ptcl);
		return -EINVAL;
	}

	if( act != OP_FUNC_ENABLE && act != OP_FUNC_DISABLE)
	{
		ip1829drv_dbg("Error: act=%X\n", act);
		return -EINVAL;
	}

	ip1829drv_dbg("cdata ->ptcl=%X\n", ptcl);
	ip1829drv_dbg("cdata ->act=%X\n", act);

	/* check L3 TCP action */
	if(_ReadRegBits(0, P0REG_L3FRAMEGETCTRL, 2, 2) == OP_CAP_ACT_DROP)
	{
		ip1829drv_dbg("Error: L3 TCP action is drop\n");
		return -EINVAL;
	}

	/* set register */
	_WriteRegBits(0, P0REG_TCPCHECKEN + (ptcl/16), (ptcl%16), 1, act);

	FUNC_MSG_OUT;
	return 0;
}

int getCosTcpEnable(void *cdata, int len)
{
	unsigned int ptcl;
	int act;

	FUNC_MSG_IN;
	if (sizeof(struct CapActSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	ptcl = ((struct CapActSetting *)cdata) ->protocol;

	if( ptcl >= OP_TCPUDP_PTCL_TOTALNUM)
	{
		ip1829drv_dbg("Error: protocol=%X\n", ptcl);
		return -EINVAL;
	}

	act = _ReadRegBits(0, P0REG_TCPCHECKEN + (ptcl/16), (ptcl%16), 1);
	((struct CapActSetting *)cdata) ->act = act;

	ip1829drv_dbg("cdata ->protocol=%X\n", ((struct CapActSetting *)cdata) ->protocol);
	ip1829drv_dbg("cdata ->act=%X\n", ((struct CapActSetting *)cdata) ->act);
	FUNC_MSG_OUT;
	return 0;
}

int setCosUdpEnable(void *cdata, int len)
{
	unsigned int ptcl;
	int act;

	FUNC_MSG_IN;
	if (sizeof(struct CapActSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	ptcl = ((struct CapActSetting *)cdata) ->protocol;
	act = ((struct CapActSetting *)cdata) ->act;

	if( ptcl >= OP_TCPUDP_PTCL_TOTALNUM)
	{
		ip1829drv_dbg("Error: protocol=%X\n", ptcl);
		return -EINVAL;
	}

	if( act != OP_FUNC_ENABLE && act != OP_FUNC_DISABLE)
	{
		ip1829drv_dbg("Error: act=%X\n", act);
		return -EINVAL;
	}

	ip1829drv_dbg("cdata ->ptcl=%X\n", ptcl);
	ip1829drv_dbg("cdata ->act=%X\n", act);

	/* check L3 UDP action */
	if(_ReadRegBits(0, P0REG_L3FRAMEGETCTRL, 4, 2) == OP_CAP_ACT_DROP)
	{
		ip1829drv_dbg("Error: L3 UDP action is drop\n");
		return -EINVAL;
	}

	/* set register */
	_WriteRegBits(0, (P0REG_UDPCHECKEN + ptcl/16), (ptcl%16), 1, act);

	FUNC_MSG_OUT;
	return 0;
}

int getCosUdpEnable(void *cdata, int len)
{
	unsigned int ptcl;
	int act;

	FUNC_MSG_IN;
	if (sizeof(struct CapActSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	ptcl = ((struct CapActSetting *)cdata) ->protocol;

	if( ptcl >= OP_TCPUDP_PTCL_TOTALNUM)
	{
		ip1829drv_dbg("Error: protocol=%X\n", ptcl);
		return -EINVAL;
	}

	act = _ReadRegBits(0, (P0REG_UDPCHECKEN + ptcl/16), (ptcl%16), 1);
	((struct CapActSetting *)cdata) ->act = act;

	ip1829drv_dbg("cdata ->protocol=%X\n", ((struct CapActSetting *)cdata) ->protocol);
	ip1829drv_dbg("cdata ->act=%X\n", ((struct CapActSetting *)cdata) ->act);
	FUNC_MSG_OUT;
	return 0;
}

int setCosTcpFlagDropNull(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _setGeneralEnable(cdata, len, 0, P0REG_TCPFLGCFGGLB, 0);
	FUNC_MSG_OUT;
	return ret;
}

int getCosTcpFlagDropNull(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _getGeneralEnable(cdata, len, 0, P0REG_TCPFLGCFGGLB, 0);
	FUNC_MSG_OUT;
	return ret;
}

int setCosTcpFlagDropAllset(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _setGeneralEnable(cdata, len, 0, P0REG_TCPFLGCFGGLB, 1);
	FUNC_MSG_OUT;
	return ret;
}

int getCosTcpFlagDropAllset(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _getGeneralEnable(cdata, len, 0, P0REG_TCPFLGCFGGLB, 1);
	FUNC_MSG_OUT;
	return ret;
}

int setCosTcpFlag(void *cdata, int len)
{
	unsigned int index;
	int fdata;

	FUNC_MSG_IN;
	if (sizeof(struct TcpFlagSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	index = ((struct TcpFlagSetting *)cdata) ->index;
	if( index > OP_TCPFLAG_FLAG3 )
	{
		ip1829drv_dbg("Error: index=%X\n", index);
		return -EINVAL;
	}

	fdata = ((struct TcpFlagSetting *)cdata) ->fdata;
	if( fdata < 0x0 || fdata > 0xF )
	{
		ip1829drv_dbg("Error: act=%X\n", fdata);
		return -EINVAL;
	}

	ip1829drv_dbg("cdata ->index=%X\n", index);
	ip1829drv_dbg("cdata ->fdata=%X\n", fdata);

	_WriteRegBits(0, P0REG_TCPFLGCFG0 + index, 0, 8, fdata);

	FUNC_MSG_OUT;
	return 0;
}

int getCosTcpFlag(void *cdata, int len)
{
	unsigned int index;
	int fdata;

	FUNC_MSG_IN;
	if (sizeof(struct TcpFlagSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	index = ((struct TcpFlagSetting *)cdata) ->index;
	if( index > OP_TCPFLAG_FLAG3 )
	{
		ip1829drv_dbg("Error: index=%X\n", index);
		return -EINVAL;
	}

	fdata = _ReadRegBits(0, P0REG_TCPFLGCFG0 + index, 0, 8);
	((struct TcpFlagSetting *)cdata) ->index = fdata;
	ip1829drv_dbg("cdata ->index=%X\n", index);
	ip1829drv_dbg("cdata ->fdata=%X\n", fdata);
	FUNC_MSG_OUT;
	return 0;
}

int setCosTcpFlagAct(void *cdata, int len)
{
	unsigned int index;
	int fdata;

	FUNC_MSG_IN;
	if (sizeof(struct TcpFlagSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	index = ((struct TcpFlagSetting *)cdata) ->index;
	if( index > OP_TCPFLAG_FLAG3 )
	{
		ip1829drv_dbg("Error: index=%X\n", index);
		return -EINVAL;
	}

	fdata = ((struct TcpFlagSetting *)cdata) ->fdata;
	if( fdata < OP_TCPFLAG_ACT_NONE || fdata > OP_TCPFLAG_ACT_DROP )
	{
		ip1829drv_dbg("Error: act=%X\n", fdata);
		return -EINVAL;
	}

	ip1829drv_dbg("cdata ->index=%X\n", index);
	ip1829drv_dbg("cdata ->fdata=%X\n", fdata);

	IP2Page(0);

	/* check L3 TCP action */
	if(_ReadRegBits(0, P0REG_L3FRAMEGETCTRL, 2, 2) == OP_CAP_ACT_DROP)
	{
		ip1829drv_dbg("Error: L3 TCP action is drop\n");
		return -EINVAL;
	}

	/* set register */
	_WriteRegBits(0, P0REG_TCPFLGCFG0 + index, 12, 2, fdata);

	FUNC_MSG_OUT;
	return 0;
}

int getCosTcpFlagAct(void *cdata, int len)
{
	unsigned int index;
	int fdata;

	FUNC_MSG_IN;
	if (sizeof(struct TcpFlagSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	index = ((struct TcpFlagSetting *)cdata) ->index;
	if( index > OP_TCPFLAG_FLAG3 )
	{
		ip1829drv_dbg("Error: index=%X\n", index);
		return -EINVAL;
	}

	fdata = _ReadRegBits(0, P0REG_TCPFLGCFG0 + index, 12, 2);
	((struct TcpFlagSetting *)cdata) ->fdata = fdata;
	ip1829drv_dbg("cdata ->index=%X\n", index);
	ip1829drv_dbg("cdata ->fdata=%X\n", fdata);
	FUNC_MSG_OUT;
	return 0;
}

int setCosTcpFlagPort(void *cdata, int len)
{
	unsigned int index;
	int fdata;

	FUNC_MSG_IN;
	if (sizeof(struct TcpFlagSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	index = ((struct TcpFlagSetting *)cdata) ->index;
	if( index > OP_TCPFLAG_FLAG3 )
	{
		ip1829drv_dbg("Error: index=%X\n", index);
		return -EINVAL;
	}

	fdata = ((struct TcpFlagSetting *)cdata) ->fdata;
	if( fdata & ~0x1FFFFFFF )
	{
		ip1829drv_dbg("Error: act=%X\n", fdata);
		return -EINVAL;
	}

	ip1829drv_dbg("cdata ->index=%X\n", index);
	ip1829drv_dbg("cdata ->fdata=%X\n", fdata);

	IP2Page(0);

	Write_Reg(P0REG_TCPFLGPORTEN+index*2+1, fdata>>16);
	Write_Reg(P0REG_TCPFLGPORTEN+index*2, fdata&0xFFFF);

	FUNC_MSG_OUT;
	return 0;
}

int getCosTcpFlagPort(void *cdata, int len)
{
	unsigned int index;
	int fdata;

	FUNC_MSG_IN;
	if (sizeof(struct TcpFlagSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	index = ((struct TcpFlagSetting *)cdata) ->index;
	if( index > OP_TCPFLAG_FLAG3 )
	{
		ip1829drv_dbg("Error: index=%X\n", index);
		return -EINVAL;
	}

	IP2Page(0);
	fdata = Read_Reg(P0REG_TCPFLGPORTEN+index*2+1)<<16 | Read_Reg(P0REG_TCPFLGPORTEN+index*2);
	((struct TcpFlagSetting *)cdata) ->fdata = fdata;
	ip1829drv_dbg("cdata ->index=%X\n", index);
	ip1829drv_dbg("cdata ->fdata=%X\n", fdata);
	FUNC_MSG_OUT;
	return 0;
}

//------------ CoS functions:common end  -----------------------
//------------ VLAN functions:common ----------------------------
int setVlanEgressFrame(void *cdata, int len)
{
	u16 mode;

	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	mode = ((struct GeneralSetting *)cdata) ->gdata;
	if (mode > 0x7 )
	{
		ip1829drv_dbg("Error: mode=%X\n", mode);
		return -EINVAL;
	}

	ip1829drv_dbg("mode=0x%08x\n", mode);

	_WriteRegBits(2, P2REG_VLAN_EGRESS_CFG1, 13, 3, mode);
	FUNC_MSG_OUT;
	return 0;
}

int getVlanEgressFrame(void *cdata, int len)
{
	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	((struct GeneralSetting *)cdata) ->gdata = (int)_ReadRegBits(2,P2REG_VLAN_EGRESS_CFG1, 13, 3);

	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	FUNC_MSG_OUT;
	return 0;
}

int setVlanTagging(void *cdata, int len)
{
	u16 mode;

	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	mode = ((struct GeneralSetting *)cdata) ->gdata;
	if ((mode != OP_VLAN_TAGGING_BY_PORT) && mode != (OP_VLAN_TAGGING_BY_VID))
	{
		ip1829drv_dbg("Error: mode=%X\n", mode);
		return -EINVAL;
	}

	ip1829drv_dbg("mode=0x%08x\n", mode);

	_WriteRegBits(2, P2REG_VLANCFG, 3, 1, mode);

	FUNC_MSG_OUT;
	return 0;
}

int getVlanTagging(void *cdata, int len)
{
	int gdata;

	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	gdata = _ReadRegBits(2, P2REG_VLANCFG, 3, 1);
	((struct GeneralSetting *)cdata) ->gdata = gdata;

	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	FUNC_MSG_OUT;
	return 0;
}

int setVlanType(void *cdata, int len)
{
	u16 mode;

	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	mode = ((struct GeneralSetting *)cdata) ->gdata;
	if ((mode != OP_VLAN_TYPE_TAG) && mode != (OP_VLAN_TYPE_GROUP))
	{
		ip1829drv_dbg("Error: mode=%X\n", mode);
		return -EINVAL;
	}

	ip1829drv_dbg("mode=0x%08x\n", mode);

	/* set vlan type: port/tag base */
	_WriteRegBits(2, P2REG_VLANCFG, 1, 1, mode);
	/* set vlan protocol base */
	_WriteRegBits(2, P2REG_VLANCFG, 0, 1, mode);

	FUNC_MSG_OUT;
	return 0;
}

int getVlanType(void *cdata, int len)
{
	int gdata;

	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	gdata = _ReadRegBits(2, P2REG_VLANCFG, 1, 1);
	((struct GeneralSetting *)cdata) ->gdata = gdata;

	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	FUNC_MSG_OUT;
	return 0;
}

int setVlanGroup(void *cdata, int len)
{
	unsigned long pm;
	int option;
	u32 u32dat;
	int i;

	FUNC_MSG_IN;
	if (sizeof(struct PortmapSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	pm = ((struct PortmapSetting *)cdata) ->portmap;
	option = ((struct PortmapSetting *)cdata) ->pmdata;
	if (pm & ~(0x1FFFFFFF))
	{
		ip1829drv_dbg("Error: portmap=%08lX\n", pm);
		return -EINVAL;
	}
	if (option & ~0x1)
	{
		ip1829drv_dbg("Error: pmdata=%X\n", option);
		return -EINVAL;
	}

	ip1829drv_dbg("pm=0x%08lx\n", pm);
	ip1829drv_dbg("option=%x\n", option);

	IP2Page(2);
	for(i=0;i<SWITCH_MAX_IPORT_CNT; i++)
	{
		if(!((pm>>i)&0x1))
			continue;

		u32dat = (u32)Read_Reg(P2REG_VLANGROUP+1+2*i)<<16 | (u32)Read_Reg(P2REG_VLANGROUP+2*i);
		if(option)
			u32dat |= pm;
		else
			u32dat &= ~pm;

		Write_Reg(P2REG_VLANGROUP+2*i, (u16)(u32dat & 0xFFFF));
		Write_Reg(P2REG_VLANGROUP+1+2*i, (u16)(u32dat >> 16));

	}
	FUNC_MSG_OUT;
	return 0;
}

int setVlanQinQPType(void *cdata, int len)
{
	int type;

	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	type = ((struct GeneralSetting *)cdata) ->gdata;
	if (type&(~0xFFFF))
	{
		ip1829drv_dbg("Error: type=%d\n", type);
		return -EINVAL;
	}

	ip1829drv_dbg("type=0x%08x\n", type);

	IP2Page(7);
	Write_Reg(P7REG_QINQEGTYPELEN, type);

	FUNC_MSG_OUT;
	return 0;
}

int getVlanQinQPType(void *cdata, int len)
{
	u16 u16dat;

	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	IP2Page(7);
	u16dat = Read_Reg(P7REG_QINQEGTYPELEN);
	((struct GeneralSetting *)cdata) ->gdata = (int)u16dat;

	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	FUNC_MSG_OUT;
	return 0;
}

int setVlanQinQPAddtag(void *cdata, int len)
{
	int member;

	FUNC_MSG_IN;
	if (sizeof(struct PortMemberSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	member = ((struct PortMemberSetting *)cdata) ->member;
	if (member & ~(0x1FFFFFFF))
	{
		ip1829drv_dbg("Error: member=%08X\n", member);
		return -EINVAL;
	}

	ip1829drv_dbg("member=0x%08x\n", member);

	IP2Page(7);
	Write_Reg(P7REG_QINQ_ADDTAG, (u16)(member & 0xFFFF));
	Write_Reg(P7REG_QINQ_ADDTAG+1, (u16)(member >> 16));
	FUNC_MSG_OUT;
	return 0;
}

int getVlanQinQPAddtag(void *cdata, int len)
{
	u32 u32dat;

	FUNC_MSG_IN;
	if (sizeof(struct PortMemberSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	IP2Page(7);
	u32dat = ((u32)Read_Reg(P7REG_QINQ_ADDTAG+1)<<16)|(u32)Read_Reg(P7REG_QINQ_ADDTAG);
	((struct PortMemberSetting *)cdata) ->member = (int)u32dat;

	ip1829drv_dbg("cdata ->gdata=%d\n", (u16)((struct PortMemberSetting *)cdata) ->member);
	FUNC_MSG_OUT;
	return 0;
}

int setVlanQinQPRmvtag(void *cdata, int len)
{
	int member;

	FUNC_MSG_IN;
	if (sizeof(struct PortMemberSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	member = ((struct PortMemberSetting *)cdata) ->member;
	if (member & ~(0x1FFFFFFF))
	{
		ip1829drv_dbg("Error: member=%08X\n", member);
		return -EINVAL;
	}

	ip1829drv_dbg("member=0x%08x\n", member);

	IP2Page(7);
	Write_Reg(P7REG_QINQ_RMVTAG, (u16)(member & 0xFFFF));
	Write_Reg(P7REG_QINQ_RMVTAG+1, (u16)(member >> 16));
	FUNC_MSG_OUT;
	return 0;
}

int getVlanQinQPRmvtag(void *cdata, int len)
{
	u32 u32dat;

	FUNC_MSG_IN;
	if (sizeof(struct PortMemberSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	IP2Page(7);
	u32dat = ((u32)Read_Reg(P7REG_QINQ_RMVTAG+1)<<16)|(u32)Read_Reg(P7REG_QINQ_RMVTAG);
	((struct PortMemberSetting *)cdata) ->member = (int)u32dat;

	ip1829drv_dbg("cdata ->gdata=%d\n", (u16)((struct PortMemberSetting *)cdata) ->member);
	FUNC_MSG_OUT;
	return 0;
}

int setVlanQinQPRxdet(void *cdata, int len)
{
	int member;

	FUNC_MSG_IN;
	if (sizeof(struct PortMemberSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	member = ((struct PortMemberSetting *)cdata) ->member;
	if (member & ~(0x1FFFFFFF))
	{
		ip1829drv_dbg("Error: member=%08X\n", member);
		return -EINVAL;
	}

	ip1829drv_dbg("member=0x%08x\n", member);

	IP2Page(7);
	Write_Reg(P7REG_QINQ_DET_RX, (u16)(member & 0xFFFF));
	Write_Reg(P7REG_QINQ_DET_RX+1, (u16)(member >> 16));
	FUNC_MSG_OUT;
	return 0;
}

int getVlanQinQPRxdet(void *cdata, int len)
{
	u32 u32dat;

	FUNC_MSG_IN;
	if (sizeof(struct PortMemberSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	IP2Page(7);
	u32dat = ((u32)Read_Reg(P7REG_QINQ_DET_RX+1)<<16)|(u32)Read_Reg(P7REG_QINQ_DET_RX);
	((struct PortMemberSetting *)cdata) ->member = (int)u32dat;

	ip1829drv_dbg("cdata ->gdata=%d\n", (u16)((struct PortMemberSetting *)cdata) ->member);
	FUNC_MSG_OUT;
	return 0;
}

int setVlanQinQPKeep(void *cdata, int len)
{
	int member;
	int i;

	FUNC_MSG_IN;
	if (sizeof(struct PortMemberSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	member = ((struct PortMemberSetting *)cdata) ->member;
	if (member & ~(0x1FFFFFFF))
	{
		ip1829drv_dbg("Error: member=%08X\n", member);
		return -EINVAL;
	}

	ip1829drv_dbg("member=0x%08x\n", member);

	for(i=0; i<SWITCH_MAX_IPORT_CNT; i++)
	{
		_WriteRegBits(7, P7REG_QINQ_P_DATA+i, 12, 1, ((member>>i)&0x1));
	}
	FUNC_MSG_OUT;
	return 0;
}

int getVlanQinQPKeep(void *cdata, int len)
{
	int member=0;
	u16 u16dat;
	int i;

	FUNC_MSG_IN;
	if (sizeof(struct PortMemberSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	IP2Page(7);
	for(i=0; i<SWITCH_MAX_IPORT_CNT; i++)
	{
		u16dat = Read_Reg(P7REG_QINQ_P_DATA+i);
		member |= ((u16dat>>12)&0x1)<<i;
	}
	((struct PortMemberSetting *)cdata) ->member = member;
	ip1829drv_dbg("cdata ->gdata=%d\n", (u16)((struct PortMemberSetting *)cdata) ->member);
	FUNC_MSG_OUT;
	return 0;
}

int setVlanQinQPIndex(void *cdata, int len)
{
	int port, index;

	FUNC_MSG_IN;
	if (sizeof(struct ByPortSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	port = ((struct ByPortSetting *)cdata) ->port;
	index= ((struct ByPortSetting *)cdata) ->pdata;
	if (port < 0 || port >= SWITCH_MAX_IPORT_CNT)
	{
		ip1829drv_dbg("Error: port=%X\n", port);
		return -EINVAL;
	}

	if (index < 0 || index > 63)
	{
		ip1829drv_dbg("Error: pmdata=%X\n", index);
		return -EINVAL;
	}

	ip1829drv_dbg("port=%d\n", port);
	ip1829drv_dbg("index=%d\n", index);

	index |= (0x0001 << 8);
	_WriteRegBits(7, P7REG_QINQ_P_DATA+port, 0, 6, index);
	FUNC_MSG_OUT;
	return 0;
}

int getVlanQinQPIndex(void *cdata, int len)
{
	int port;
	int index;

	FUNC_MSG_IN;
	if (sizeof(struct ByPortSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	port = ((struct ByPortSetting *)cdata) ->port;
	if (port < 0 || port >= SWITCH_MAX_IPORT_CNT)
	{
		ip1829drv_dbg("Error: port=%X\n", port);
		return -EINVAL;
	}

	index= _ReadRegBits(7,P7REG_QINQ_P_DATA+port, 0, 6);

	((struct ByPortSetting *)cdata) ->pdata = index;
	ip1829drv_dbg("cdata ->pdata=%d\n", ((struct ByPortSetting *)cdata) ->pdata);
	FUNC_MSG_OUT;
	return 0;
}

int setVlanQinQIndex(void *cdata, int len)
{
	int index, data;

	FUNC_MSG_IN;
	if (sizeof(struct VlanSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	index = ((struct VlanSetting *)cdata) ->vtype;
	data= ((struct VlanSetting *)cdata) ->vdata;
	if (index < 0 || index > 63)
	{
		ip1829drv_dbg("Error: vtype=%X\n", index);
		return -EINVAL;
	}

	if (data & (~0xFFFF))
	{
		ip1829drv_dbg("Error: vdata=%X\n", data);
		return -EINVAL;
	}

	ip1829drv_dbg("index=%d\n", index);
	ip1829drv_dbg("data=%d\n", data);

	IP2Page(7);

	Write_Reg(P7REG_QINQ_DATA+index, (u16)data);

	FUNC_MSG_OUT;
	return 0;
}

int getVlanQinQIndex(void *cdata, int len)
{
	int index, data;

	FUNC_MSG_IN;
	if (sizeof(struct VlanSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	index = ((struct VlanSetting *)cdata) ->vtype;
	if (index < 0 || index > 63)
	{
		ip1829drv_dbg("Error: vtype=%X\n", index);
		return -EINVAL;
	}

	IP2Page(7);
	data= Read_Reg(P7REG_QINQ_DATA+index);
	((struct VlanSetting *)cdata) ->vdata = data;
	ip1829drv_dbg("cdata ->pdata=%d\n", ((struct VlanSetting *)cdata) ->vdata);
	FUNC_MSG_OUT;
	return 0;
}

int setVlanPortAddtag(void *cdata, int len)
{
	int member;

	FUNC_MSG_IN;
	if (sizeof(struct PortMemberSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	member = ((struct PortMemberSetting *)cdata) ->member;
	if (member & ~(0x1FFFFFFF))
	{
		ip1829drv_dbg("Error: member=%08X\n", member);
		return -EINVAL;
	}

	ip1829drv_dbg("member=0x%08x\n", member);

	IP2Page(2);
	Write_Reg(P2REG_VLAN_ADDTAG, (u16)(member & 0xFFFF));
	Write_Reg(P2REG_VLAN_ADDTAG+1, (u16)(member >> 16));
	FUNC_MSG_OUT;
	return 0;
}

int getVlanPortAddtag(void *cdata, int len)
{
	u32 u32dat;

	FUNC_MSG_IN;
	if (sizeof(struct PortMemberSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	IP2Page(2);
	u32dat = ((u32)Read_Reg(P2REG_VLAN_ADDTAG+1)<<16)|(u32)Read_Reg(P2REG_VLAN_ADDTAG);
	((struct PortMemberSetting *)cdata) ->member = (int)u32dat;

	ip1829drv_dbg("cdata ->gdata=%d\n", (u16)((struct PortMemberSetting *)cdata) ->member);
	FUNC_MSG_OUT;
	return 0;
}

int setVlanPortRmvtag(void *cdata, int len)
{
	int member;

	FUNC_MSG_IN;
	if (sizeof(struct PortMemberSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	member = ((struct PortMemberSetting *)cdata) ->member;
	if (member & ~(0x1FFFFFFF))
	{
		ip1829drv_dbg("Error: member=%08X\n", member);
		return -EINVAL;
	}

	ip1829drv_dbg("member=0x%08x\n", member);

	IP2Page(2);
	Write_Reg(P2REG_VLAN_RMVTAG, (u16)(member & 0xFFFF));
	Write_Reg(P2REG_VLAN_RMVTAG+1, (u16)(member >> 16));
	FUNC_MSG_OUT;
	return 0;
}

int getVlanPortRmvtag(void *cdata, int len)
{
	u32 u32dat;

	FUNC_MSG_IN;
	if (sizeof(struct PortMemberSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	IP2Page(2);
	u32dat = ((u32)Read_Reg(P2REG_VLAN_RMVTAG+1)<<16)|(u32)Read_Reg(P2REG_VLAN_RMVTAG);
	((struct PortMemberSetting *)cdata) ->member = (int)u32dat;

	ip1829drv_dbg("cdata ->gdata=%d\n", (u16)((struct PortMemberSetting *)cdata) ->member);
	FUNC_MSG_OUT;
	return 0;
}

int setVlanPortForce(void *cdata, int len)
{
	int member;
	int i;

	FUNC_MSG_IN;
	if (sizeof(struct PortMemberSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	member = ((struct PortMemberSetting *)cdata) ->member;
	if (member & ~(0x1FFFFFFF))
	{
		ip1829drv_dbg("Error: member=%08X\n", member);
		return -EINVAL;
	}

	ip1829drv_dbg("member=0x%08x\n", member);

	for(i=0; i<SWITCH_MAX_IPORT_CNT; i++)
	{
		_WriteRegBits(2, P2REG_VLAN_PVIDCFG+i, 15, 1, (member>>i)&0x1);
	}
	FUNC_MSG_OUT;
	return 0;
}

int getVlanPortForce(void *cdata, int len)
{
	int member=0;
	u16 u16dat;
	int i;

	FUNC_MSG_IN;
	if (sizeof(struct PortMemberSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	IP2Page(2);
	for(i=0; i<SWITCH_MAX_IPORT_CNT; i++)
	{
		u16dat = Read_Reg(P2REG_VLAN_PVIDCFG+i);
		member |= ((u16dat>>15)&0x1)<<i;
	}
	((struct PortMemberSetting *)cdata) ->member = member;
	ip1829drv_dbg("cdata ->gdata=%d\n", (u16)((struct PortMemberSetting *)cdata) ->member);
	FUNC_MSG_OUT;
	return 0;
}

int setVlanPortUplink(void *cdata, int len)
{
	int member;
	u16 u16dat;

	FUNC_MSG_IN;
	if (sizeof(struct PortMemberSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	member = ((struct PortMemberSetting *)cdata) ->member;
	if (member & ~(0x1FFFFFFF))
	{
		ip1829drv_dbg("Error: member=%08X\n", member);
		return -EINVAL;
	}

	ip1829drv_dbg("member=0x%08x\n", member);

	IP2Page(2);
	Write_Reg(P2REG_VLAN_UPLINK, (u16)(member & 0xFFFF));
	u16dat = (u16)(member >> 16);
	u16dat |= (u16)0x1<<13;		//enable uplink
	_WriteRegBits(2, P2REG_VLAN_UPLINK+1, 0, 14, u16dat);
	FUNC_MSG_OUT;
	return 0;
}

int getVlanPortUplink(void *cdata, int len)
{
	u32 u32dat;

	FUNC_MSG_IN;
	if (sizeof(struct PortMemberSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	IP2Page(2);
	u32dat = ((u32)Read_Reg(P2REG_VLAN_UPLINK+1)<<16)|(u32)Read_Reg(P2REG_VLAN_UPLINK);
	u32dat &= 0x1FFFFFFF;
	((struct PortMemberSetting *)cdata) ->member = (int)u32dat;

	ip1829drv_dbg("cdata ->gdata=%d\n", (u16)((struct PortMemberSetting *)cdata) ->member);
	FUNC_MSG_OUT;
	return 0;
}

int setVlanPortExclusive(void *cdata, int len)
{
	int member;

	FUNC_MSG_IN;
	if (sizeof(struct PortMemberSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	member = ((struct PortMemberSetting *)cdata) ->member;
	if (member & ~(0x1FFFFFFF))
	{
		ip1829drv_dbg("Error: member=%08X\n", member);
		return -EINVAL;
	}

	ip1829drv_dbg("member=0x%08x\n", member);

	IP2Page(2);
	Write_Reg(P2REG_VLAN_EXCLUSIVE, (u16)(member & 0xFFFF));
	Write_Reg(P2REG_VLAN_EXCLUSIVE+1, (u16)(member >> 16));
	FUNC_MSG_OUT;
	return 0;
}

int getVlanPortExclusive(void *cdata, int len)
{
	u32 u32dat;

	FUNC_MSG_IN;
	if (sizeof(struct PortMemberSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	IP2Page(2);
	u32dat = ((u32)Read_Reg(P2REG_VLAN_EXCLUSIVE+1)<<16)|(u32)Read_Reg(P2REG_VLAN_EXCLUSIVE);
	((struct PortMemberSetting *)cdata) ->member = (int)u32dat;

	ip1829drv_dbg("cdata ->gdata=%d\n", (u16)((struct PortMemberSetting *)cdata) ->member);
	FUNC_MSG_OUT;
	return 0;
}

int setVlanPortEgress(void *cdata, int len)
{
	int member;

	FUNC_MSG_IN;
	if (sizeof(struct PortMemberSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	member = ((struct PortMemberSetting *)cdata) ->member;
	if (member & ~(0x1FFFFFFF))
	{
		ip1829drv_dbg("Error: member=%08X\n", member);
		return -EINVAL;
	}

	ip1829drv_dbg("member=0x%08x\n", member);

	IP2Page(2);
	Write_Reg(P2REG_VLAN_EGRESS_CFG, (u16)(member & 0xFFFF));

	_WriteRegBits(2, P2REG_VLAN_EGRESS_CFG+1, 0, 13, (u16)(member >> 16));


	FUNC_MSG_OUT;
	return 0;
}

int getVlanPortEgress(void *cdata, int len)
{
	u32 u32dat;

	FUNC_MSG_IN;
	if (sizeof(struct PortMemberSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	IP2Page(2);
	u32dat = ((u32)Read_Reg(P2REG_VLAN_EGRESS_CFG+1)<<16)|(u32)Read_Reg(P2REG_VLAN_EGRESS_CFG);
	u32dat &= 0x1FFFFFFF;
	((struct PortMemberSetting *)cdata) ->member = (int)u32dat;

	ip1829drv_dbg("cdata ->gdata=%d\n", (u16)((struct PortMemberSetting *)cdata) ->member);
	FUNC_MSG_OUT;
	return 0;
}

int setVlanPortIngressFrame(void *cdata, int len)
{
	int port, pdata;

	FUNC_MSG_IN;
	if (sizeof(struct ByPortSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	port = ((struct ByPortSetting *)cdata) ->port;
	pdata = ((struct ByPortSetting *)cdata) ->pdata;

	if (port < 0 || port >= SWITCH_MAX_IPORT_CNT)
	{
		ip1829drv_dbg("Error: port=%d\n", port);
		return -EINVAL;
	}

	if (pdata < 0 || pdata > 0x3)
	{
		ip1829drv_dbg("Error: pdata=0x%X\n", pdata);
		return -EINVAL;
	}

	ip1829drv_dbg("port=%d\n", port);
	ip1829drv_dbg("pdata=0x%X\n", pdata);

	_WriteRegBits(2, P2REG_VLAN_INGRESS_FRAME+(port/8), (port%8)*2, 2, pdata);

	FUNC_MSG_OUT;
	return 0;
}

int getVlanPortIngressFrame(void *cdata, int len)
{
	int port, pdata;

	FUNC_MSG_IN;
	if (sizeof(struct ByPortSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	port = ((struct ByPortSetting *)cdata) ->port;

	if (port < 0 || port >= SWITCH_MAX_IPORT_CNT)
	{
		ip1829drv_dbg("Error: port=%d\n", port);
		return -EINVAL;
	}

	ip1829drv_dbg("port=%d\n", port);

	pdata = _ReadRegBits(2, P2REG_VLAN_INGRESS_FRAME+(port/8), (port%8)*2, 2);
	((struct ByPortSetting *)cdata) ->pdata = pdata;

	FUNC_MSG_OUT;
	return 0;
}

int setVlanPortIngressCheck(void *cdata, int len)
{
	int member;

	FUNC_MSG_IN;
	if (sizeof(struct PortMemberSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	member = ((struct PortMemberSetting *)cdata) ->member;
	if (member & ~(0x1FFFFFFF))
	{
		ip1829drv_dbg("Error: member=%08X\n", member);
		return -EINVAL;
	}

	ip1829drv_dbg("member=0x%08x\n", member);

	IP2Page(2);
	Write_Reg(P2REG_VLAN_INGRESS_CHK, (u16)(member & 0xFFFF));
	Write_Reg(P2REG_VLAN_INGRESS_CHK+1, (u16)(member >> 16));
	FUNC_MSG_OUT;
	return 0;
}

int getVlanPortIngressCheck(void *cdata, int len)
{
	u32 u32dat;

	FUNC_MSG_IN;
	if (sizeof(struct PortMemberSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	IP2Page(2);
	u32dat = ((u32)Read_Reg(P2REG_VLAN_INGRESS_CHK+1)<<16)|(u32)Read_Reg(P2REG_VLAN_INGRESS_CHK);
	((struct PortMemberSetting *)cdata) ->member = (int)u32dat;

	ip1829drv_dbg("cdata ->gdata=%d\n", (u16)((struct PortMemberSetting *)cdata) ->member);
	FUNC_MSG_OUT;
	return 0;
}

int setVlanPortMember(void *cdata, int len)
{
	int port, member;

	FUNC_MSG_IN;
	if (sizeof(struct ByPortSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	port = ((struct ByPortSetting *)cdata) ->port;
	member= ((struct ByPortSetting *)cdata) ->pdata;
	if (port < 0 || port >= SWITCH_MAX_IPORT_CNT)
	{
		ip1829drv_dbg("Error: port=%X\n", port);
		return -EINVAL;
	}

	if (member & ~(0x1FFFFFFF))
	{
		ip1829drv_dbg("Error: pmdata=%X\n", member);
		return -EINVAL;
	}

	ip1829drv_dbg("port=%d\n", port);
	ip1829drv_dbg("member=%d\n", member);

	IP2Page(2);

	Write_Reg(P2REG_VLANGROUP+2*port, (u16)(member & 0xFFFF));
	Write_Reg(P2REG_VLANGROUP+1+2*port, (u16)(member >> 16));

	FUNC_MSG_OUT;
	return 0;
}

int getVlanPortMember(void *cdata, int len)
{
	int port;
	u32 u32dat;

	FUNC_MSG_IN;
	if (sizeof(struct ByPortSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	port = ((struct ByPortSetting *)cdata) ->port;
	if (port < 0 || port >= SWITCH_MAX_IPORT_CNT)
	{
		ip1829drv_dbg("Error: port=%X\n", port);
		return -EINVAL;
	}

	IP2Page(2);
	u32dat= ((u32)Read_Reg(P2REG_VLANGROUP+1+2*port)<<16)|(u32)Read_Reg(P2REG_VLANGROUP+2*port);
	((struct ByPortSetting *)cdata) ->pdata = u32dat;
	ip1829drv_dbg("cdata ->pdata=%d\n", ((struct ByPortSetting *)cdata) ->pdata);
	FUNC_MSG_OUT;
	return 0;
}

int setVlanPortVid(void *cdata, int len)
{
	int port, pvid;

	FUNC_MSG_IN;
	if (sizeof(struct ByPortSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	port = ((struct ByPortSetting *)cdata) ->port;
	pvid= ((struct ByPortSetting *)cdata) ->pdata;
	if (port < 0 || port >= SWITCH_MAX_IPORT_CNT)
	{
		ip1829drv_dbg("Error: port=%X\n", port);
		return -EINVAL;
	}

	if (pvid < 0 || pvid >= MAX_PVID_NUM)
	{
		ip1829drv_dbg("Error: pmdata=%X\n", pvid);
		return -EINVAL;
	}

	ip1829drv_dbg("port=%d\n", port);
	ip1829drv_dbg("pvid=%d\n", pvid);

	_WriteRegBits(2, P2REG_VLAN_PVIDCFG+port, 0, 12, pvid);
	FUNC_MSG_OUT;
	return 0;
}

int getVlanPortVid(void *cdata, int len)
{
	int port, pvid;

	FUNC_MSG_IN;
	if (sizeof(struct ByPortSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	port = ((struct ByPortSetting *)cdata) ->port;
	if (port < 0 || port >= SWITCH_MAX_IPORT_CNT)
	{
		ip1829drv_dbg("Error: port=%X\n", port);
		return -EINVAL;
	}

	pvid= _ReadRegBits(2, P2REG_VLAN_PVIDCFG+port, 0, 12);

	((struct ByPortSetting *)cdata) ->pdata = pvid;
	ip1829drv_dbg("cdata ->pdata=%d\n", ((struct ByPortSetting *)cdata) ->pdata);
	FUNC_MSG_OUT;
	return 0;
}

int setVlanProtocolMode(void *cdata, int len)
{
	int index, mode;

	FUNC_MSG_IN;
	if (sizeof(struct VlanSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	index = (((struct VlanSetting *)cdata) ->vtype) - 1;
	mode= ((struct VlanSetting *)cdata) ->vdata;
	if (index < 0 || index >= MAX_PRO_VLAN_ENTRY_NUM)
	{
		ip1829drv_dbg("Error: vtype=%X\n", index);
		return -EINVAL;
	}

	if(	(mode!=OP_VLAN_PROTOCOL_INVALID)
			&&	(mode!=OP_VLAN_PROTOCOL_ETHER)
			&&	(mode!=OP_VLAN_PROTOCOL_LLC)
			&&	(mode!=OP_VLAN_PROTOCOL_1042) )
	{
		ip1829drv_dbg("Error: vdata=%X\n", mode);
		return -EINVAL;
	}

	ip1829drv_dbg("index=%d\n", index);
	ip1829drv_dbg("mode=%d\n", mode);

	_WriteRegBits(2, P2REG_VLAN_PROCOTOL_CFG+1+index*2, 12, 2, mode);
	FUNC_MSG_OUT;
	return 0;
}

int getVlanProtocolMode(void *cdata, int len)
{
	int index, mode;

	FUNC_MSG_IN;
	if (sizeof(struct VlanSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	index = (((struct VlanSetting *)cdata) ->vtype) - 1;
	if (index < 0 || index >= MAX_PRO_VLAN_ENTRY_NUM)
	{
		ip1829drv_dbg("Error: vtype=%X\n", index);
		return -EINVAL;
	}

	mode = (int)_ReadRegBits(2, P2REG_VLAN_PROCOTOL_CFG+1+index*2, 12, 2);

	((struct VlanSetting *)cdata) ->vdata = mode;
	ip1829drv_dbg("cdata ->pdata=%d\n", ((struct VlanSetting *)cdata) ->vdata);
	FUNC_MSG_OUT;
	return 0;
}

int setVlanProtocolVid(void *cdata, int len)
{
	int index, vid;

	FUNC_MSG_IN;
	if (sizeof(struct VlanSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	index = (((struct VlanSetting *)cdata) ->vtype) - 1;
	vid= ((struct VlanSetting *)cdata) ->vdata;
	if (index < 0 || index >= MAX_PRO_VLAN_ENTRY_NUM)
	{
		ip1829drv_dbg("Error: vtype=%X\n", index);
		return -EINVAL;
	}

	if(	(vid < 0) || (vid>=MAX_PVID_NUM) )
	{
		ip1829drv_dbg("Error: vdata=%X\n", vid);
		return -EINVAL;
	}

	ip1829drv_dbg("index=%d\n", index);
	ip1829drv_dbg("vid=%d\n", vid);

	_WriteRegBits(2, P2REG_VLAN_PROCOTOL_CFG+1+index*2, 0, 12, vid);
	FUNC_MSG_OUT;
	return 0;
}

int getVlanProtocolVid(void *cdata, int len)
{
	int index, vid;

	FUNC_MSG_IN;
	if (sizeof(struct VlanSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	index = (((struct VlanSetting *)cdata) ->vtype) - 1;
	if (index < 0 || index >= MAX_PRO_VLAN_ENTRY_NUM)
	{
		ip1829drv_dbg("Error: vtype=%X\n", index);
		return -EINVAL;
	}

	vid = (int)_ReadRegBits(2, P2REG_VLAN_PROCOTOL_CFG+1+index*2, 0, 12);

	((struct VlanSetting *)cdata) ->vdata = vid;
	ip1829drv_dbg("cdata ->pdata=%d\n", ((struct VlanSetting *)cdata) ->vdata);
	FUNC_MSG_OUT;
	return 0;
}

int setVlanProtocolType(void *cdata, int len)
{
	int index, type;

	FUNC_MSG_IN;
	if (sizeof(struct VlanSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	index = (((struct VlanSetting *)cdata) ->vtype) - 1;
	type= ((struct VlanSetting *)cdata) ->vdata;
	if (index < 0 || index >= MAX_PRO_VLAN_ENTRY_NUM)
	{
		ip1829drv_dbg("Error: vtype=%X\n", index);
		return -EINVAL;
	}

	if(	type&(~0xFFFF) )
	{
		ip1829drv_dbg("Error: vdata=%X\n", type);
		return -EINVAL;
	}

	ip1829drv_dbg("index=%d\n", index);
	ip1829drv_dbg("type=%d\n", type);

	IP2Page(2);

	Write_Reg(P2REG_VLAN_PROCOTOL_CFG+index*2, (u16)type);

	FUNC_MSG_OUT;
	return 0;
}

int getVlanProtocolType(void *cdata, int len)
{
	int index, type;

	FUNC_MSG_IN;
	if (sizeof(struct VlanSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	index = (((struct VlanSetting *)cdata) ->vtype) - 1;
	if (index < 0 || index >= MAX_PRO_VLAN_ENTRY_NUM)
	{
		ip1829drv_dbg("Error: vtype=%X\n", index);
		return -EINVAL;
	}

	IP2Page(2);
	type = (int)Read_Reg(P2REG_VLAN_PROCOTOL_CFG+index*2);
	((struct VlanSetting *)cdata) ->vdata = type;
	ip1829drv_dbg("cdata ->pdata=%d\n", ((struct VlanSetting *)cdata) ->vdata);
	FUNC_MSG_OUT;
	return 0;
}

int setVlanProtocolClear(void *cdata, int len)
{
	int index;

	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	index = (((struct GeneralSetting *)cdata) ->gdata) - 1;
	if (index < 0 || index >= MAX_PRO_VLAN_ENTRY_NUM)
	{
		ip1829drv_dbg("Error: vtype=%X\n", index);
		return -EINVAL;
	}

	ip1829drv_dbg("index=0x%08x\n", index);

	IP2Page(2);
	Write_Reg(P2REG_VLAN_PROCOTOL_CFG+index*2, 0 );
	Write_Reg(P2REG_VLAN_PROCOTOL_CFG+index*2+1, 0 );
	FUNC_MSG_OUT;
	return 0;
}

int setVlanEntryMember(void *cdata, int len)
{
	int vid, member;
	u16 u16dat;

	FUNC_MSG_IN;
	if (sizeof(struct PortmapSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	vid = ((struct PortmapSetting *)cdata) ->pmdata;
	member= ((struct PortmapSetting *)cdata) ->portmap;
	if (vid < 0 || vid >= MAX_PVID_NUM)
	{
		ip1829drv_dbg("Error: vtype=%X\n", vid);
		return -EINVAL;
	}

	if (member & ~(0x1FFFFFFF))
	{
		ip1829drv_dbg("Error: vdata=%X\n", member);
		return -EINVAL;
	}

	ip1829drv_dbg("vid=%d\n", vid);
	ip1829drv_dbg("member=%d\n", member);

	IP2Page(2);
	/* read data from VLAN table */
	u16dat = 0x8000 | vid;
	Write_Reg(P2REG_VLANCMD, u16dat);
	/* wait for command ack */
	while(Read_Reg(P2REG_VLANCMD)&0x8000);


	/* set entry memeber */
	u16dat = (u16)member&0xFFFF;
	Write_Reg(P2REG_VLANDAT0, u16dat);

	u16dat = Read_Reg(P2REG_VLANDAT0+1);
	u16dat &= ~0x1FFF;
	u16dat |= (u16)(member>>16)&0x1FFF;
	Write_Reg(P2REG_VLANDAT0+1, u16dat);

	/* set valid bit to 1 */
	u16dat = Read_Reg(P2REG_VLANDAT0+5);
	u16dat |= (u16)0x1<<15;
	Write_Reg(P2REG_VLANDAT0+5, u16dat);

	/* Write data to VLAN table */
	u16dat = 0xC000 | vid;
	Write_Reg(P2REG_VLANCMD, u16dat);
	/* wait for command ack */
	while(Read_Reg(P2REG_VLANCMD)&0x8000);

	FUNC_MSG_OUT;
	return 0;
}

int getVlanEntryMember(void *cdata, int len)
{
	int vid, member;
	u16 u16dat;

	FUNC_MSG_IN;
	if (sizeof(struct PortmapSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	vid = ((struct PortmapSetting *)cdata) ->pmdata;
	if (vid < 0 || vid >= MAX_PVID_NUM)
	{
		ip1829drv_dbg("Error: vtype=%X\n", vid);
		return -EINVAL;
	}

	ip1829drv_dbg("vid=%d\n", vid);

	IP2Page(2);
	/* read data from VLAN table */
	u16dat = 0x8000 | vid;
	Write_Reg(P2REG_VLANCMD, u16dat);
	/* wait for command ack */
	while(Read_Reg(P2REG_VLANCMD)&0x8000);

	/* get entry memeber */
	member = (((u32)Read_Reg(P2REG_VLANDAT0+1)<<16) | (u32)Read_Reg(P2REG_VLANDAT0))&0x1FFFFFFF;

	((struct PortmapSetting *)cdata) ->portmap = member;
	ip1829drv_dbg("cdata ->portmap=%d\n", (u16)((struct PortmapSetting *)cdata) ->portmap);
	FUNC_MSG_OUT;
	return 0;
}

int setVlanEntryAddtag(void *cdata, int len)
{
	int vid, addtag;
	u16 u16dat;

	FUNC_MSG_IN;
	if (sizeof(struct PortmapSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	vid = ((struct PortmapSetting *)cdata) ->pmdata;
	addtag= ((struct PortmapSetting *)cdata) ->portmap;
	if (vid < 0 || vid >= MAX_PVID_NUM)
	{
		ip1829drv_dbg("Error: pmdata=%X\n", vid);
		return -EINVAL;
	}

	if (addtag & ~(0x1FFFFFFF))
	{
		ip1829drv_dbg("Error: portmap=%X\n", addtag);
		return -EINVAL;
	}

	ip1829drv_dbg("vid=%d\n", vid);
	ip1829drv_dbg("addtag=%d\n", addtag);

	IP2Page(2);
	/* read data from VLAN table */
	u16dat = 0x8000 | vid;
	Write_Reg(P2REG_VLANCMD, u16dat);
	/* wait for command ack */
	while(Read_Reg(P2REG_VLANCMD)&0x8000);


	/* set entry addtag */
	u16dat = Read_Reg(P2REG_VLANDAT0+1);
	u16dat &= ~((u16)0x7<<13);
	u16dat |= (u16)(addtag&0x7)<<13;
	Write_Reg(P2REG_VLANDAT0+1, u16dat);

	u16dat = (u16)(addtag>>3)&0xFFFF;
	Write_Reg(P2REG_VLANDAT0+2, u16dat);

	u16dat = Read_Reg(P2REG_VLANDAT0+3);
	u16dat &= ~0x03FF;
	u16dat |= (u16)(addtag>>19)&0x03FF;
	Write_Reg(P2REG_VLANDAT0+3, u16dat);

	/* set valid bit to 1 */
	u16dat = Read_Reg(P2REG_VLANDAT0+5);
	u16dat |= (u16)0x1<<15;
	Write_Reg(P2REG_VLANDAT0+5, u16dat);

	/* Write data to VLAN table */
	u16dat = 0xC000 | vid;
	Write_Reg(P2REG_VLANCMD, u16dat);
	/* wait for command ack */
	while(Read_Reg(P2REG_VLANCMD)&0x8000);

	FUNC_MSG_OUT;
	return 0;
}

int getVlanEntryAddtag(void *cdata, int len)
{
	int vid, addtag=0;
	u16 u16dat;

	FUNC_MSG_IN;
	if (sizeof(struct PortmapSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	vid = ((struct PortmapSetting *)cdata) ->pmdata;
	if (vid < 0 || vid >= MAX_PVID_NUM)
	{
		ip1829drv_dbg("Error: pmdata=%X\n", vid);
		return -EINVAL;
	}

	ip1829drv_dbg("vid=%d\n", vid);

	IP2Page(2);
	/* read data from VLAN table */
	u16dat = 0x8000 | vid;
	Write_Reg(P2REG_VLANCMD, u16dat);
	/* wait for command ack */
	while(Read_Reg(P2REG_VLANCMD)&0x8000);

	/* get entry addtag */
	u16dat = Read_Reg(P2REG_VLANDAT0+1);
	addtag |= (int)((u16dat>>13)&0x7);

	u16dat = Read_Reg(P2REG_VLANDAT0+2);
	addtag |= (int)u16dat<<3;

	u16dat = Read_Reg(P2REG_VLANDAT0+3);
	addtag |= (int)(u16dat&0x3FF)<<19;

	((struct PortmapSetting *)cdata) ->portmap = addtag;
	ip1829drv_dbg("cdata ->pmdata=%d\n", (u16)((struct PortmapSetting *)cdata) ->portmap);
	FUNC_MSG_OUT;
	return 0;
}

int setVlanEntryRmvtag(void *cdata, int len)
{
	int vid, rmvtag;
	u16 u16dat;

	FUNC_MSG_IN;
	if (sizeof(struct PortmapSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	vid = ((struct PortmapSetting *)cdata) ->pmdata;
	rmvtag= ((struct PortmapSetting *)cdata) ->portmap;
	if (vid < 0 || vid >= MAX_PVID_NUM)
	{
		ip1829drv_dbg("Error: pmdata=%X\n", vid);
		return -EINVAL;
	}

	if (rmvtag & ~(0x1FFFFFFF))
	{
		ip1829drv_dbg("Error: portmap=%X\n", rmvtag);
		return -EINVAL;
	}

	ip1829drv_dbg("vid=%d\n", vid);
	ip1829drv_dbg("rmvtag=%d\n", rmvtag);

	IP2Page(2);
	/* read data from VLAN table */
	u16dat = 0x8000 | vid;
	Write_Reg(P2REG_VLANCMD, u16dat);
	/* wait for command ack */
	while(Read_Reg(P2REG_VLANCMD)&0x8000);


	/* set entry rmvtag */
	u16dat = Read_Reg(P2REG_VLANDAT0+3);
	u16dat &= ~((u16)0x3F<<10);
	u16dat |= (u16)(rmvtag&0x3F)<<10;
	Write_Reg(P2REG_VLANDAT0+3, u16dat);

	u16dat = (u16)(rmvtag>>6)&0xFFFF;
	Write_Reg(P2REG_VLANDAT0+4, u16dat);

	u16dat = Read_Reg(P2REG_VLANDAT0+5);
	u16dat &= ~0x7F;
	u16dat |= (u16)(rmvtag>>22)&0x7F;

	/* set valid bit to 1 */
	u16dat |= (u16)0x1<<15;
	Write_Reg(P2REG_VLANDAT0+5, u16dat);

	/* Write data to VLAN table */
	u16dat = 0xC000 | vid;
	Write_Reg(P2REG_VLANCMD, u16dat);
	/* wait for command ack */
	while(Read_Reg(P2REG_VLANCMD)&0x8000);

	FUNC_MSG_OUT;
	return 0;
}

int getVlanEntryRmvtag(void *cdata, int len)
{
	int vid, rmvtag=0;
	u16 u16dat;

	FUNC_MSG_IN;
	if (sizeof(struct PortmapSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	vid = ((struct PortmapSetting *)cdata) ->pmdata;
	if (vid < 0 || vid >= MAX_PVID_NUM)
	{
		ip1829drv_dbg("Error: pmdata=%X\n", vid);
		return -EINVAL;
	}

	ip1829drv_dbg("vid=%d\n", vid);

	IP2Page(2);
	/* read data from VLAN table */
	u16dat = 0x8000 | vid;
	Write_Reg(P2REG_VLANCMD, u16dat);
	/* wait for command ack */
	while(Read_Reg(P2REG_VLANCMD)&0x8000);

	/* get entry rmvtag */
	u16dat = Read_Reg(P2REG_VLANDAT0+3);
	rmvtag |= (int)((u16dat>>10)&0x3F);

	u16dat = Read_Reg(P2REG_VLANDAT0+4);
	rmvtag |= (int)u16dat<<6;

	u16dat = Read_Reg(P2REG_VLANDAT0+5);
	rmvtag |= (int)(u16dat&0x7F)<<22;

	((struct PortmapSetting *)cdata) ->portmap = rmvtag;
	ip1829drv_dbg("cdata ->pmdata=%d\n", (u16)((struct PortmapSetting *)cdata) ->portmap);
	FUNC_MSG_OUT;
	return 0;
}

int setVlanEntryPriority(void *cdata, int len)
{
	int vid, priority;
	u16 u16dat;

	FUNC_MSG_IN;
	if (sizeof(struct VlanSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	vid = ((struct VlanSetting *)cdata) ->vtype;
	priority= ((struct VlanSetting *)cdata) ->vdata;
	if (vid < 0 || vid >= MAX_PVID_NUM)
	{
		ip1829drv_dbg("Error: vtype=%X\n", vid);
		return -EINVAL;
	}

	if (priority & ~(0xF))
	{
		ip1829drv_dbg("Error: vdata=%X\n", priority);
		return -EINVAL;
	}

	ip1829drv_dbg("vid=%d\n", vid);
	ip1829drv_dbg("priority=%d\n", priority);

	IP2Page(2);
	/* read data from VLAN table */
	u16dat = 0x8000 | vid;
	Write_Reg(P2REG_VLANCMD, u16dat);
	/* wait for command ack */
	while(Read_Reg(P2REG_VLANCMD)&0x8000);


	/* set entry priority */
	u16dat = Read_Reg(P2REG_VLANDAT0+5);
	u16dat &= ~((u16)0xF<<7);
	u16dat |= (u16)(priority&0xF)<<7;

	/* set valid bit to 1 */
	u16dat |= (u16)0x1<<15;
	Write_Reg(P2REG_VLANDAT0+5, u16dat);

	/* Write data to VLAN table */
	u16dat = 0xC000 | vid;
	Write_Reg(P2REG_VLANCMD, u16dat);
	/* wait for command ack */
	while(Read_Reg(P2REG_VLANCMD)&0x8000);

	FUNC_MSG_OUT;
	return 0;
}

int getVlanEntryPriority(void *cdata, int len)
{
	int vid, priority=0;
	u16 u16dat;

	FUNC_MSG_IN;
	if (sizeof(struct VlanSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	vid = ((struct VlanSetting *)cdata) ->vtype;
	if (vid < 0 || vid >= MAX_PVID_NUM)
	{
		ip1829drv_dbg("Error: vtype=%X\n", vid);
		return -EINVAL;
	}

	ip1829drv_dbg("vid=%d\n", vid);

	IP2Page(2);
	/* read data from VLAN table */
	u16dat = 0x8000 | vid;
	Write_Reg(P2REG_VLANCMD, u16dat);
	/* wait for command ack */
	while(Read_Reg(P2REG_VLANCMD)&0x8000);

	/* get entry priority */
	u16dat = Read_Reg(P2REG_VLANDAT0+5);
	priority = (int)(u16dat>>7)&0xF;

	((struct VlanSetting *)cdata) ->vdata = priority;
	ip1829drv_dbg("cdata ->pdata=%d\n", ((struct VlanSetting *)cdata) ->vdata);
	FUNC_MSG_OUT;
	return 0;
}
int setVlanEntryFid(void *cdata, int len)
{
	int vid, fid;
	u16 u16dat;

	FUNC_MSG_IN;
	if (sizeof(struct VlanSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	vid = ((struct VlanSetting *)cdata) ->vtype;
	fid= ((struct VlanSetting *)cdata) ->vdata;
	if (vid < 0 || vid >= MAX_PVID_NUM)
	{
		ip1829drv_dbg("Error: vtype=%X\n", vid);
		return -EINVAL;
	}

	if (fid & ~(0xF))
	{
		ip1829drv_dbg("Error: vdata=%X\n", fid);
		return -EINVAL;
	}

	ip1829drv_dbg("vid=%d\n", vid);
	ip1829drv_dbg("fid=%d\n", fid);

	IP2Page(2);
	/* read data from VLAN table */
	u16dat = 0x8000 | vid;
	Write_Reg(P2REG_VLANCMD, u16dat);
	/* wait for command ack */
	while(Read_Reg(P2REG_VLANCMD)&0x8000);


	/* set entry fid */
	u16dat = Read_Reg(P2REG_VLANDAT0+5);
	u16dat &= ~((u16)0xF<<11);
	u16dat |= (u16)(fid&0xF)<<11;

	/* set valid bit to 1 */
	u16dat |= (u16)0x1<<15;
	Write_Reg(P2REG_VLANDAT0+5, u16dat);

	/* Write data to VLAN table */
	u16dat = 0xC000 | vid;
	Write_Reg(P2REG_VLANCMD, u16dat);
	/* wait for command ack */
	while(Read_Reg(P2REG_VLANCMD)&0x8000);

	FUNC_MSG_OUT;
	return 0;
}

int getVlanEntryFid(void *cdata, int len)
{
	int vid, fid=0;
	u16 u16dat;

	FUNC_MSG_IN;
	if (sizeof(struct VlanSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	vid = ((struct VlanSetting *)cdata) ->vtype;
	if (vid < 0 || vid >= MAX_PVID_NUM)
	{
		ip1829drv_dbg("Error: vtype=%X\n", vid);
		return -EINVAL;
	}

	ip1829drv_dbg("vid=%d\n", vid);

	IP2Page(2);
	/* read data from VLAN table */
	u16dat = 0x8000 | vid;
	Write_Reg(P2REG_VLANCMD, u16dat);
	/* wait for command ack */
	while(Read_Reg(P2REG_VLANCMD)&0x8000);

	/* get entry fid */
	u16dat = Read_Reg(P2REG_VLANDAT0+5);
	fid = (int)(u16dat>>11)&0xF;

	((struct VlanSetting *)cdata) ->vdata = fid;
	ip1829drv_dbg("cdata ->pdata=%d\n", ((struct VlanSetting *)cdata) ->vdata);
	FUNC_MSG_OUT;
	return 0;
}

int setVlanEntryClear(void *cdata, int len)
{
	int vid;
	u16 u16dat;
	int i;

	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	vid = ((struct GeneralSetting *)cdata) ->gdata;
	if (vid < 0 || vid >= MAX_PVID_NUM)
	{
		ip1829drv_dbg("Error: vtype=%X\n", vid);
		return -EINVAL;
	}

	ip1829drv_dbg("vid=%d\n", vid);

	IP2Page(2);

	/* clear entry data */
	for(i=0; i<5; i++)
		Write_Reg(P2REG_VLANDAT0+i, 0 );

	/* Write data from VLAN table */
	u16dat = 0xC000 | vid;
	Write_Reg(P2REG_VLANCMD, u16dat);
	/* wait for command ack */
	while(Read_Reg(P2REG_VLANCMD)&0x8000);

	FUNC_MSG_OUT;
	return 0;
}

//------------ VLAN functions:common end  -----------------------
//------------ MIB Counter functions:ip1829 ---------------------
int setMibCounterEnable(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _setGeneralEnable(cdata, len, 0, P0REG_MACBEHAVIOR, 9);
	FUNC_MSG_OUT;
	return ret;
}
int getMibCounterEnable(void *cdata, int len)
{
	int ret=0;
	FUNC_MSG_IN;
	ret = _getGeneralEnable(cdata, len, 0, P0REG_MACBEHAVIOR, 9);
	FUNC_MSG_OUT;
	return ret;
}
int getMibCounterAll(void *cdata, int len)
{
	int i,j,ret=0;
	void *uptr;
	struct MIBCounterEntry_all *dptr;

	FUNC_MSG_IN;
	dptr = kmalloc(sizeof(struct MIBCounterEntry_all), GFP_KERNEL);
	if (!dptr)
	{
		ret =  -ENOMEM;
		goto out_mib_counter_all;
	}

	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		ret = -EINVAL;
		goto out_mib_counter_all;
	}

	uptr = (void *)(((struct GeneralSetting *)cdata) ->gdata);

	IP2Page(0);
	for(j=0;j<SWITCH_MAX_IPORT_CNT;j++)
	{
		for(i=0;i<NUM_MIB_COUNTER_RX;i++)
		{
			//statistic counter read , read clear rx counter
			Write_Reg(POREG_MIBCOUN_CMD,0xC000|(u16)i|((u16)j<<5));
			/* need to check does trigger bit has been put down */
			while((Read_Reg(POREG_MIBCOUN_CMD) >> 15)&0x1);
			//dptr ->RX_counter[i]=(u32)(((u32)Read_Reg(POREG_MIBCOUN_DATA_H)<<16)|Read_Reg(POREG_MIBCOUN_DATA_L));
			dptr ->entry[j].RX_counter[i] =  (u32)Read_Reg(POREG_MIBCOUN_DATA_L);
			dptr ->entry[j].RX_counter[i] |= ((u32)Read_Reg(POREG_MIBCOUN_DATA_H)<<16);
		}
		for(i=0;i<NUM_MIB_COUNTER_TX;i++)
		{
			//statistic counter read , read clear tx counter
			Write_Reg(POREG_MIBCOUN_CMD,0xC400|(u16)i|((u16)j<<5));
			/* need to check does trigger bit has been put down */
			while((Read_Reg(POREG_MIBCOUN_CMD) >> 15)&0x1);
			//dptr ->TX_counter[i]=(u32)(((u32)Read_Reg(POREG_MIBCOUN_DATA_H)<<16)|Read_Reg(POREG_MIBCOUN_DATA_L));
			dptr ->entry[j].TX_counter[i] =  (u32)Read_Reg(POREG_MIBCOUN_DATA_L);
			dptr ->entry[j].TX_counter[i] |= ((u32)Read_Reg(POREG_MIBCOUN_DATA_H)<<16);
		}
	}

	if (copy_to_user(uptr, dptr, sizeof(struct MIBCounterEntry_all)))
	{
		ret = -EFAULT;
		goto out_mib_counter_all;
	}
out_mib_counter_all:
	if(dptr)
	{ kfree(dptr);}
	FUNC_MSG_OUT;
	return (ret < 0) ? ret : 0;
}
int getMibCounterByPort(void *cdata, int len)
{
	int i,ret=0,port;
	void *uptr;
	struct MIBCounterEntry *dptr;
	FUNC_MSG_IN;

	dptr = kmalloc(sizeof(struct MIBCounterEntry), GFP_KERNEL);
	if (!dptr)
	{
		ret = -ENOMEM;
		goto out_mib_counter;
	}

	if (sizeof(struct ByPortSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		ret = -EINVAL;
		goto out_mib_counter;
	}

	uptr = (void *)(((struct ByPortSetting *)cdata) ->pdata);
	port = ((struct ByPortSetting *)cdata) ->port;
	IP2Page(0);

	for(i=0;i<NUM_MIB_COUNTER_RX;i++)
	{
		//statistic counter read , read clear rx counter
		Write_Reg(POREG_MIBCOUN_CMD,0xC000|(u16)i|((u16)port<<5));
		/* need to check does trigger bit has been put down */
		while((Read_Reg(POREG_MIBCOUN_CMD) >> 15)&0x1);
		dptr ->RX_counter[i]=(u32)(((u32)Read_Reg(POREG_MIBCOUN_DATA_H)<<16)|Read_Reg(POREG_MIBCOUN_DATA_L));
	}
	for(i=0;i<NUM_MIB_COUNTER_TX;i++)
	{
		//statistic counter read , read clear tx counter
		Write_Reg(POREG_MIBCOUN_CMD,0xC400|(u16)i|((u16)port<<5));
		/* need to check does trigger bit has been put down */
		while((Read_Reg(POREG_MIBCOUN_CMD) >> 15)&0x1);
		dptr ->TX_counter[i]=(u32)(((u32)Read_Reg(POREG_MIBCOUN_DATA_H)<<16)|Read_Reg(POREG_MIBCOUN_DATA_L));
	}
	if (copy_to_user(uptr, dptr, sizeof(struct MIBCounterEntry)))
	{
		ret = -EFAULT;
		goto out_mib_counter;
	}
out_mib_counter:
	if(dptr)
	{ kfree(dptr);}
	FUNC_MSG_OUT;
	return (ret < 0) ? ret : 0;
}

int getMibCounterByItem(void *cdata, int len)
{
	int port, dir, idx;
	FUNC_MSG_IN;

	if (sizeof(struct MIBCounterData) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	port = (((struct MIBCounterData *)cdata) ->port) - 1;
	dir = ((struct MIBCounterData *)cdata) ->dir;
	idx = ((struct MIBCounterData *)cdata) ->idx;

	IP2Page(0);

	//statistic counter read , read clear rx counter
	Write_Reg(POREG_MIBCOUN_CMD,(dir?0xC400:0xC000)|(u16)idx|((u16)port<<5));
	/* need to check does trigger bit has been put down */
	while((Read_Reg(POREG_MIBCOUN_CMD) >> 15)&0x1);

	((struct MIBCounterData *)cdata) ->counter=(u32)(((u32)Read_Reg(POREG_MIBCOUN_DATA_H)<<16)|Read_Reg(POREG_MIBCOUN_DATA_L));
	FUNC_MSG_OUT;
	return 0;

}

//------------ MIB Counter functions:ip1829 end------------------
//------------ QOS functions:ip1829 -----------------------------
int setQOSAgingFunction(void *cdata, int len)
{
	int port,aging;
	FUNC_MSG_IN;
	if (sizeof(struct ByPortSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	port=((struct ByPortSetting *)cdata) ->port;
	aging =((struct ByPortSetting *)cdata) ->pdata;
	if (port < 1 || port > SWITCH_MAX_IPORT_CNT)
		return -EINVAL;
	if (aging < 0 || aging > 0xff)
		return -EINVAL;

	_WriteRegBits(8, P8REG_QOSPORTAGINGEN0+((port-1)/2), ((port-1)%2)*8, 8, aging);

	FUNC_MSG_OUT;
	return 0;
}
int getQOSAgingFunction(void *cdata, int len)
{
	int port;
	FUNC_MSG_IN;
	if (sizeof(struct ByPortSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}
	port=((struct ByPortSetting *)cdata) ->port;
	if (port < 1 || port > SWITCH_MAX_IPORT_CNT)
		return -EINVAL;

	((struct ByPortSetting *)cdata) ->pdata=(u32)_ReadRegBits(8,P8REG_QOSPORTAGINGEN0+((port-1)/2),((port-1)%2)*8,8);

	FUNC_MSG_OUT;
	ip1829drv_dbg("cdata ->pdata=%d\n", ((struct ByPortSetting *)cdata) ->pdata);
	return 0;
}
int setQOSAgingTime(void *cdata, int len)
{
	int aging;
	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}
	aging = ((struct GeneralSetting *)cdata) ->gdata;
	if (aging < 0 || aging > 0xff)
		return -EINVAL;

	_WriteRegBits(8, P8REG_QOSAGINGTIME, 0, 8, aging);
	FUNC_MSG_OUT;
	return 0;
}
int getQOSAgingTime(void *cdata, int len)
{
	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	((struct GeneralSetting *)cdata) ->gdata = (u16)_ReadRegBits(8, P8REG_QOSAGINGTIME, 0, 8);

	FUNC_MSG_OUT;
	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	return 0;
}
int setQOSFastAging(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _setGeneralEnable(cdata, len, 8, P8REG_QOSAGINGTIME, 8);
	FUNC_MSG_OUT;
	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	return ret;
}
int getQOSFastAging(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _getGeneralEnable(cdata, len, 8, P8REG_QOSAGINGTIME, 8);
	FUNC_MSG_OUT;
	ip1829drv_dbg("cdata ->gdata=%d\n", ((struct GeneralSetting *)cdata) ->gdata);
	return ret;
}

int setCOSACL(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _setPortmap(cdata, len, 0, P0REG_QOSACLBASEPRIEN0);
	FUNC_MSG_OUT;
	return ret;
}
int getCOSACL(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _getPortmap(cdata, len, 0, P0REG_QOSACLBASEPRIEN0);
	FUNC_MSG_OUT;
	return ret;
}
int setCOSIGMP(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _setPortmap(cdata, len, 0, P0REG_QOSIGMPBASEPRIEN0);
	FUNC_MSG_OUT;
	return ret;
}
int getCOSIGMP(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _getPortmap(cdata, len, 0, P0REG_QOSIGMPBASEPRIEN0);
	FUNC_MSG_OUT;
	return ret;
}
int setCOSIPAddress(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _setPortmap(cdata, len, 0, P0REG_QOSIPBASEPRIEN0);
	FUNC_MSG_OUT;
	return ret;
}
int getCOSIPAddress(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _getPortmap(cdata, len, 0, P0REG_QOSIPBASEPRIEN0);
	FUNC_MSG_OUT;
	return ret;
}
int setCOSMACAddress(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _setPortmap(cdata, len, 0, P0REG_QOSMACBASEPRIEN0);
	FUNC_MSG_OUT;
	return ret;
}
int getCOSMACAddress(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _getPortmap(cdata, len, 0, P0REG_QOSMACBASEPRIEN0);
	FUNC_MSG_OUT;
	return ret;
}
int setCOSVID(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _setPortmap(cdata, len, 0, P0REG_QOSVIDBASEPRIEN0);
	FUNC_MSG_OUT;
	return ret;
}
int getCOSVID(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _getPortmap(cdata, len, 0, P0REG_QOSVIDBASEPRIEN0);
	FUNC_MSG_OUT;
	return ret;
}
int setCOSTCPUDPPort(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _setPortmap(cdata, len, 0, P0REG_QOSTCPUDPBASEPRIEN0);
	FUNC_MSG_OUT;
	return ret;
}
int getCOSTCPUDPPort(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _getPortmap(cdata, len, 0, P0REG_QOSTCPUDPBASEPRIEN0);
	FUNC_MSG_OUT;
	return ret;
}
int setCOSDSCP(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _setPortmap(cdata, len, 0, P0REG_QOSDSCPBASEPRIEN0);
	FUNC_MSG_OUT;
	return ret;
}
int getCOSDSCP(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _getPortmap(cdata, len, 0, P0REG_QOSDSCPBASEPRIEN0);
	FUNC_MSG_OUT;
	return ret;
}
int setCOS8021P(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _setPortmap(cdata, len, 0, P0REG_QOS8021PBASEPRIEN0);
	FUNC_MSG_OUT;
	return ret;
}
int getCOS8021P(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _getPortmap(cdata, len, 0, P0REG_QOS8021PBASEPRIEN0);
	FUNC_MSG_OUT;
	return ret;
}
int setCOSPhsicalPort(void *cdata, int len)
{
	int port,pdata;

	FUNC_MSG_IN;
	if (sizeof(struct ByPortSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	port = ((struct ByPortSetting *)cdata) ->port;
	pdata = ((struct ByPortSetting *)cdata) ->pdata;
	if (port < 1 || port > SWITCH_MAX_IPORT_CNT)
		return -EINVAL;
	if( pdata < OP_QOS_NUM_Q0 || pdata > OP_QOS_NUM_Q7 )
	{
		ip1829drv_dbg("Error: pdata=%X\n", pdata);
		return -EINVAL;
	}

	ip1829drv_dbg("cdata ->pdata=%d\n", pdata);

	port-=1;
	_WriteRegBits(0, P0REG_QOSPORTBASEPRIQUE0+(port/5), (port%5)*3, 3, pdata);
	FUNC_MSG_OUT;
	return 0;
}
int getCOSPhsicalPort(void *cdata, int len)
{
	int port;
	FUNC_MSG_IN;
	if (sizeof(struct ByPortSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}
	port = ((struct ByPortSetting *)cdata) ->port;
	if (port < 1 || port > SWITCH_MAX_IPORT_CNT)
		return -EINVAL;
	port-=1;
	((struct ByPortSetting *)cdata) ->pdata = (int)((_ReadRegBits(0,P0REG_QOSPORTBASEPRIQUE0+(port/5),(port%5)*3,3)));

	FUNC_MSG_OUT;
	return 0;
}
int setQOS8021PEdtion(void *cdata, int len)
{
	int gdata;
	u16 regdata;

	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	gdata = ((struct GeneralSetting *)cdata) ->gdata;
	if( gdata != OP_QOS_8021PEDTION_2005 && gdata != OP_QOS_8021PEDTION_2005_EX && gdata != OP_QOS_8021PEDTION_EARLY )
	{
		ip1829drv_dbg("Error: gdata=%X\n", gdata);
		return -EINVAL;
	}

	ip1829drv_dbg("cdata ->gdata=%d\n", gdata);
	IP2Page(0);
	regdata = Read_Reg(P0REG_QOS8021PBASEPRIEN1);
	regdata &= (u16)~0xC000;
	switch(gdata)
	{
		case OP_QOS_8021PEDTION_2005:
			break;
		case OP_QOS_8021PEDTION_2005_EX:
			regdata |= 0x8000;  break;
		case OP_QOS_8021PEDTION_EARLY:
			regdata |= 0x4000;  break;
	}
	Write_Reg(P0REG_QOS8021PBASEPRIEN1,regdata);

	FUNC_MSG_OUT;
	return 0;
}
int getQOS8021PEdtion(void *cdata, int len)
{
	u16 u16dat;
	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}
	u16dat =_ReadRegBits(0, P0REG_QOS8021PBASEPRIEN1, 14, 2);

	if(u16dat&0x1)
	{ ((struct GeneralSetting *)cdata) ->gdata = OP_QOS_8021PEDTION_EARLY; }
	else
	{
		if(u16dat&0x2)
		{ ((struct GeneralSetting *)cdata) ->gdata = OP_QOS_8021PEDTION_2005_EX; }
		else
		{ ((struct GeneralSetting *)cdata) ->gdata = OP_QOS_8021PEDTION_2005; }
	}
	ip1829drv_dbg("cdata ->gdata=0x%08x\n", ((struct GeneralSetting *)cdata) ->gdata);
	FUNC_MSG_OUT;

	FUNC_MSG_OUT;
	return 0;
}
int setQOSDSCPBaseDSCP(void *cdata, int len)
{
	u8 entryno,value,queue;
	FUNC_MSG_IN;
	if (sizeof(struct qos_dscp_setting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}

	entryno = ((struct qos_dscp_setting *)cdata) ->dscpentry;
	value = ((struct qos_dscp_setting *)cdata) ->dscpvalue;
	queue = ((struct qos_dscp_setting *)cdata) ->dscpqueue;

	if(entryno<1 || entryno>8)
	{
		ip1829drv_dbg("Error: entryno=%d\n", entryno);
		return -EINVAL;
	}
	if(value & ~0x3f)
	{
		ip1829drv_dbg("Error: value=%X\n", value);
		return -EINVAL;
	}
	if(queue>7)
	{
		ip1829drv_dbg("Error: queue=%d\n", queue);
		return -EINVAL;
	}

	entryno-=1;

	_WriteRegBits(0, P0REG_QOSDSCPVALUE0+(entryno/2), (entryno%2)*6, 6, value);
	_WriteRegBits(0, P0REG_QOSDSCPPRISETTING0+(entryno/5), (entryno%5)*3, 3, queue);
	FUNC_MSG_OUT;
	return 0;
}
int getQOSDSCPBaseDSCP(void *cdata, int len)
{
	u8 entryno;
	FUNC_MSG_IN;
	if (sizeof(struct qos_dscp_setting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}
	entryno = ((struct qos_dscp_setting *)cdata) ->dscpentry;
	if(entryno>7)
	{
		ip1829drv_dbg("Error: entryno=%d\n", entryno);
		return -EINVAL;
	}
	((struct qos_dscp_setting *)cdata) ->dscpvalue = (u16)_ReadRegBits(0, P0REG_QOSDSCPVALUE0+(entryno/2), (entryno%2)*5, 6);
	((struct qos_dscp_setting *)cdata) ->dscpqueue = (u16)_ReadRegBits(0, P0REG_QOSDSCPPRISETTING0+(entryno/5), (entryno%5)*3, 3);
	ip1829drv_dbg("cdata ->dscpvalue=0x%04X\n", ((struct qos_dscp_setting *)cdata) ->dscpvalue);
	ip1829drv_dbg("cdata ->dscpqueue=0x%04X\n", ((struct qos_dscp_setting *)cdata) ->dscpqueue);
	FUNC_MSG_OUT;

	return 0;
}
int setQOSDSCPBaseNoMatchAction(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _setGeneralEnable(cdata, len, 0, P0REG_QOSDSCPPRISETTING1, 9);
	FUNC_MSG_OUT;
	return ret;
}
int getQOSDSCPBaseNoMatchAction(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _getGeneralEnable(cdata, len, 0, P0REG_QOSDSCPPRISETTING1, 9);
	FUNC_MSG_OUT;
	return ret;
}

int setQOSmodeGroupMember(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _setPortmap(cdata, len, 8, P8REG_QOSGROUPSEL0);
	FUNC_MSG_OUT;
	return ret;
}
int getQOSmodeGroupMember(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _getPortmap(cdata, len, 8, P8REG_QOSGROUPSEL0);
	FUNC_MSG_OUT;
	return ret;
}
int setQOSGroupBEn(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _setGeneralEnable(cdata, len, 8, P8REG_QOSMODESELGROUP2, 9);
	FUNC_MSG_OUT;
	return ret;
}
int getQOSGroupBEn(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _getGeneralEnable(cdata, len, 8, P8REG_QOSMODESELGROUP2, 9);
	FUNC_MSG_OUT;
	return ret;
}
int setQOSMode(void *cdata, int len)
{
	int gpnum,mode;
	FUNC_MSG_IN;
	if (sizeof(struct qos_modesettings) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}
	gpnum = ((struct qos_modesettings *)cdata) ->groupnum;
	mode = ((struct qos_modesettings *)cdata) ->modesettings;
	if(gpnum!=OP_QOS_GROUP1 && gpnum!=OP_QOS_GROUP2)
	{
		ip1829drv_dbg("Error: gpnum=%d\n", gpnum);
		return -EINVAL;
	}
	if(mode<OP_QOS_MODE_FIFO || mode>OP_QOS_MODE_SP8)
	{
		ip1829drv_dbg("Error: mode=%d\n", mode);
		return -EINVAL;
	}
	_WriteRegBits(8, (gpnum==OP_QOS_GROUP1)?P8REG_QOSMODESELGROUP1:P8REG_QOSMODESELGROUP2, 0, 3, mode);


	FUNC_MSG_OUT;
	return 0;
}
int getQOSMode(void *cdata, int len)
{
	int gpnum;
	FUNC_MSG_IN;
	if (sizeof(struct qos_modesettings) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}
	gpnum = ((struct qos_modesettings *)cdata) ->groupnum;
	if(gpnum!=OP_QOS_GROUP1 && gpnum!=OP_QOS_GROUP2)
	{
		ip1829drv_dbg("Error: gpnum=%d\n", gpnum);
		return -EINVAL;
	}

	((struct qos_modesettings *)cdata) ->modesettings= (int)_ReadRegBits(8,(gpnum==OP_QOS_GROUP1)?P8REG_QOSMODESELGROUP1:P8REG_QOSMODESELGROUP2, 0, 3);

	ip1829drv_dbg("cdata ->groupnum=0x%04X\n", ((struct qos_modesettings *)cdata) ->groupnum);
	ip1829drv_dbg("cdata ->modesettings=0x%04X\n", ((struct qos_modesettings *)cdata) ->modesettings);
	FUNC_MSG_OUT;

	return 0;
}
int setQOSMethod(void *cdata, int len)
{
	int gpnum,mode;
	u16 u16dat;
	FUNC_MSG_IN;
	if (sizeof(struct qos_modesettings) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}
	gpnum = ((struct qos_modesettings *)cdata) ->groupnum;
	mode = ((struct qos_modesettings *)cdata) ->modesettings;
	if(gpnum!=OP_QOS_GROUP1 && gpnum!=OP_QOS_GROUP2)
	{
		ip1829drv_dbg("Error: gpnum=%d\n", gpnum);
		return -EINVAL;
	}
	if(mode<OP_QOS_METHOD_WRR || mode>OP_QOS_METHOD_TWRR)
	{
		ip1829drv_dbg("Error: mode=%d\n", mode);
		return -EINVAL;
	}
	IP2Page(8);

	u16dat = Read_Reg((gpnum==OP_QOS_GROUP1)?P8REG_QOSMODESELGROUP1:P8REG_QOSMODESELGROUP2);
	u16dat &= 0xFF9F;
	switch(mode)
	{
		case OP_QOS_METHOD_WRR:
			break;
		case OP_QOS_METHOD_BW:
			u16dat |= ((u16)0x1<<5);
			break;
		case OP_QOS_METHOD_WFQ:
			u16dat |= ((u16)0x2<<5);
			break;
		case OP_QOS_METHOD_TWRR:
			u16dat |= ((u16)0x3<<5);
			break;
	}
	Write_Reg((gpnum==OP_QOS_GROUP1)?P8REG_QOSMODESELGROUP1:P8REG_QOSMODESELGROUP2,u16dat);


	if(mode==OP_QOS_METHOD_BW)
	{
		Write_Reg(P8REG_QOSMODESELGROUP1,Read_Reg(P8REG_QOSMODESELGROUP1)|0x8000);//stop & hold pkt
		Write_Reg(P8REG_QOSQUEUEDBMEN0,0xFFFF);//enable DBM
		Write_Reg(P8REG_QOSQUEUEDBMEN1,0x1FFF);
	}

	ip1829drv_dbg("cdata ->groupnum=0x%04X\n", ((struct qos_modesettings *)cdata) ->groupnum);
	ip1829drv_dbg("cdata ->modesettings=0x%04X\n", ((struct qos_modesettings *)cdata) ->modesettings);
	FUNC_MSG_OUT;
	return 0;
}
int getQOSMethod(void *cdata, int len)
{
	int gpnum;
	u16 u16dat;
	FUNC_MSG_IN;
	if (sizeof(struct qos_modesettings) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}
	gpnum = ((struct qos_modesettings *)cdata) ->groupnum;
	if(gpnum!=OP_QOS_GROUP1 && gpnum!=OP_QOS_GROUP2)
	{
		ip1829drv_dbg("Error: gpnum=%d\n", gpnum);
		return -EINVAL;
	}

	u16dat = _ReadRegBits(8, (gpnum==OP_QOS_GROUP1)?P8REG_QOSMODESELGROUP1:P8REG_QOSMODESELGROUP2, 5, 3);

	((struct qos_modesettings *)cdata) ->modesettings = u16dat;

	ip1829drv_dbg("cdata ->groupnum=0x%04X\n", ((struct qos_modesettings *)cdata) ->groupnum);
	ip1829drv_dbg("cdata ->modesettings=0x%04X\n", ((struct qos_modesettings *)cdata) ->modesettings);
	FUNC_MSG_OUT;
	return 0;
}
int setQOSWeight(void *cdata, int len)
{
	int gpnum,mode,queue;
	FUNC_MSG_IN;
	if (sizeof(struct qos_modesettings) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}
	gpnum = ((struct qos_modesettings *)cdata) ->groupnum;
	queue = ((struct qos_modesettings *)cdata) ->queuenum;
	mode = ((struct qos_modesettings *)cdata) ->modesettings;
	if(gpnum!=OP_QOS_GROUP1 && gpnum!=OP_QOS_GROUP2)
	{
		ip1829drv_dbg("Error: gpnum=%d\n", gpnum);
		return -EINVAL;
	}
	if(queue<OP_QOS_NUM_Q0 || queue>OP_QOS_NUM_Q7)
	{
		ip1829drv_dbg("Error: queue=%d\n", queue);
		return -EINVAL;
	}
	if(mode & ~(int)0xff)
	{
		ip1829drv_dbg("Error: mode=%d\n", mode);
		return -EINVAL;
	}

	_WriteRegBits(8, (gpnum==OP_QOS_GROUP1)?(P8REG_QOSGP1_WEIGHT0+(queue/2)):(P8REG_QOSGP2_WEIGHT0+(queue/2)), (queue%2)*8, 8, mode);

	FUNC_MSG_OUT;
	return 0;
}
int getQOSWeight(void *cdata, int len)
{
	int gpnum,queue;
	u16 u16dat;
	FUNC_MSG_IN;
	if (sizeof(struct qos_modesettings) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}
	gpnum = ((struct qos_modesettings *)cdata) ->groupnum;
	queue = ((struct qos_modesettings *)cdata) ->queuenum;
	if(gpnum!=OP_QOS_GROUP1 && gpnum!=OP_QOS_GROUP2)
	{
		ip1829drv_dbg("Error: gpnum=%d\n", gpnum);
		return -EINVAL;
	}
	u16dat = _ReadRegBits(8,(gpnum==OP_QOS_GROUP1)?(P8REG_QOSGP1_WEIGHT0+(queue/2)):(P8REG_QOSGP2_WEIGHT0+(queue/2)), ((queue%2)*8), 8);

	((struct qos_modesettings *)cdata) ->modesettings=(int)u16dat;

	ip1829drv_dbg("cdata ->groupnum=0x%04X\n", ((struct qos_modesettings *)cdata) ->groupnum);
	ip1829drv_dbg("cdata ->queuenum=0x%04X\n", ((struct qos_modesettings *)cdata) ->queuenum);
	ip1829drv_dbg("cdata ->modesettings=0x%04X\n", ((struct qos_modesettings *)cdata) ->modesettings);
	FUNC_MSG_OUT;

	return 0;
}
int setQOSMaxBandwidth(void *cdata, int len)
{
	int gpnum,mode,queue;
	FUNC_MSG_IN;
	if (sizeof(struct qos_modesettings) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}
	gpnum = ((struct qos_modesettings *)cdata) ->groupnum;
	queue = ((struct qos_modesettings *)cdata) ->queuenum;
	mode = ((struct qos_modesettings *)cdata) ->modesettings;
	if(gpnum!=OP_QOS_GROUP1 && gpnum!=OP_QOS_GROUP2)
	{
		ip1829drv_dbg("Error: gpnum=%d\n", gpnum);
		return -EINVAL;
	}
	if(gpnum!=OP_QOS_GROUP1 && gpnum!=OP_QOS_GROUP2)
	{
		ip1829drv_dbg("Error: gpnum=%d\n", gpnum);
		return -EINVAL;
	}
	if(queue<OP_QOS_NUM_Q0 || queue>OP_QOS_NUM_Q7)
	{
		ip1829drv_dbg("Error: queue=%d\n", queue);
		return -EINVAL;
	}
	if(mode & ~(int)0xff)
	{
		ip1829drv_dbg("Error: mode=%d\n", mode);
		return -EINVAL;
	}

	_WriteRegBits(8, (gpnum==OP_QOS_GROUP1)?(P8REG_QOSGP1_MAXBDWT0+(queue/2)):(P8REG_QOSGP2_MAXBDWT0+(queue/2)), (queue%2)*8, 8, mode);

	FUNC_MSG_OUT;
	return 0;
}

int getQOSMaxBandwidth(void *cdata, int len)
{
	int gpnum,queue;
	u16 u16dat;
	FUNC_MSG_IN;
	if (sizeof(struct qos_modesettings) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}
	gpnum = ((struct qos_modesettings *)cdata) ->groupnum;
	queue = ((struct qos_modesettings *)cdata) ->queuenum;
	if(gpnum!=OP_QOS_GROUP1 && gpnum!=OP_QOS_GROUP2)
	{
		ip1829drv_dbg("Error: gpnum=%d\n", gpnum);
		return -EINVAL;
	}
	if(gpnum!=OP_QOS_GROUP1 && gpnum!=OP_QOS_GROUP2)
	{
		ip1829drv_dbg("Error: gpnum=%d\n", gpnum);
		return -EINVAL;
	}

	u16dat = _ReadRegBits(8, (gpnum==OP_QOS_GROUP1)?(P8REG_QOSGP1_MAXBDWT0+(queue/2)):(P8REG_QOSGP2_MAXBDWT0+(queue/2)), ((queue%2)*8), 8);

	((struct qos_modesettings *)cdata) ->modesettings=(int)u16dat;

	ip1829drv_dbg("cdata ->groupnum=0x%04X\n", ((struct qos_modesettings *)cdata) ->groupnum);
	ip1829drv_dbg("cdata ->queuenum=0x%04X\n", ((struct qos_modesettings *)cdata) ->queuenum);
	ip1829drv_dbg("cdata ->modesettings=0x%04X\n", ((struct qos_modesettings *)cdata) ->modesettings);
	FUNC_MSG_OUT;

	return 0;
}
int setQOSUnit(void *cdata, int len)
{
	int gpnum,mode;
	FUNC_MSG_IN;
	if (sizeof(struct qos_modesettings) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}
	gpnum = ((struct qos_modesettings *)cdata) ->groupnum;
	mode = ((struct qos_modesettings *)cdata) ->modesettings;
	if(gpnum!=OP_QOS_GROUP1 && gpnum!=OP_QOS_GROUP2)
	{
		ip1829drv_dbg("Error: gpnum=%d\n", gpnum);
		return -EINVAL;
	}
	if(mode<OP_QOS_UNIT_64KBS || mode>OP_QOS_UNIT_4MBS)
	{
		ip1829drv_dbg("Error: mode=%d\n", mode);
		return -EINVAL;
	}
	_WriteRegBits(8, (gpnum==OP_QOS_GROUP1)?(P8REG_QOSMODESELGROUP1):(P8REG_QOSMODESELGROUP2), 3, 2, mode);

	FUNC_MSG_OUT;
	return 0;
}
int getQOSUnit(void *cdata, int len)
{
	int gpnum;
	FUNC_MSG_IN;
	if (sizeof(struct qos_modesettings) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}
	gpnum = ((struct qos_modesettings *)cdata) ->groupnum;
	if(gpnum!=OP_QOS_GROUP1 && gpnum!=OP_QOS_GROUP2)
	{
		ip1829drv_dbg("Error: gpnum=%d\n", gpnum);
		return -EINVAL;
	}

	((struct qos_modesettings *)cdata) ->modesettings= (int)_ReadRegBits(8,(gpnum==OP_QOS_GROUP1)?P8REG_QOSMODESELGROUP1:P8REG_QOSMODESELGROUP2, 3, 2);

	ip1829drv_dbg("cdata ->groupnum=0x%04X\n", ((struct qos_modesettings *)cdata) ->groupnum);
	ip1829drv_dbg("cdata ->modesettings=0x%04X\n", ((struct qos_modesettings *)cdata) ->modesettings);
	FUNC_MSG_OUT;
	return 0;
}
int setQOSRatioValue0Def(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _setGeneralEnable(cdata, len, 8, P8REG_QOSMODESELGROUP1, 15);
	FUNC_MSG_OUT;
	return ret;
}
int getQOSRatioValue0Def(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _getGeneralEnable(cdata, len, 8, P8REG_QOSMODESELGROUP1, 15);
	FUNC_MSG_OUT;
	return ret;
}
int setQOSSBMDBM(void *cdata, int len)
{
	int port,mode;
	u16 u16dat;
	FUNC_MSG_IN;
	if (sizeof(struct ByPortSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}
	port = ((struct ByPortSetting *)cdata) ->port;
	mode = ((struct ByPortSetting *)cdata) ->pdata;
	if(port<1 || port>SWITCH_MAX_IPORT_CNT)
		return -EINVAL;
	if(mode!=OP_QOS_QBASE_DBM && mode!=OP_QOS_QBASE_SBM)
	{
		ip1829drv_dbg("Error: mode=%d\n", mode);
		return -EINVAL;
	}
	ip1829drv_dbg(" port=%d\n", port);
	ip1829drv_dbg(" mode=%d\n", mode);
	port-=1;
	IP2Page(8);

	// set all queue(per port) to same mode
	u16dat=Read_Reg(P8REG_QOS_SBMDBMSEL0+(port/2));
	u16dat&=((port%2)?0x00FF:0xFF00);
	if(mode==OP_QOS_QBASE_SBM)
	{ u16dat|=((port%2)?0xFF00:0x00FF); }
	Write_Reg(P8REG_QOS_SBMDBMSEL0+(port/2),u16dat);

	FUNC_MSG_OUT;
	return 0;
}
int getQOSSBMDBM(void *cdata, int len)
{
	int port;
	u16 u16dat;
	FUNC_MSG_IN;
	if (sizeof(struct ByPortSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}
	port = ((struct ByPortSetting *)cdata) ->port;
	if(port<1 || port>SWITCH_MAX_IPORT_CNT)
		return -EINVAL;
	port-=1;
	IP2Page(8);
	u16dat=Read_Reg(P8REG_QOS_SBMDBMSEL0+(port/2));
	u16dat>>=((port%2)*8);
	if(u16dat&0xff)
	{ ((struct ByPortSetting *)cdata) ->pdata=OP_QOS_QBASE_SBM;  }
	else
	{ ((struct ByPortSetting *)cdata) ->pdata=OP_QOS_QBASE_DBM;  }

	ip1829drv_dbg("cdata ->port=0x%d\n", ((struct ByPortSetting *)cdata) ->port);
	ip1829drv_dbg("cdata ->pdata=0x%d\n", ((struct ByPortSetting *)cdata) ->pdata);
	FUNC_MSG_OUT;

	return 0;
}
int setQOSDBMEn(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _setPortmap(cdata, len, 8, P8REG_QOSQUEUEDBMEN0);
	FUNC_MSG_IN;
	return ret;
}
int getQOSDBMEn(void *cdata, int len)
{
	int ret;
	FUNC_MSG_IN;
	ret = _getPortmap(cdata, len, 8, P8REG_QOSQUEUEDBMEN0);
	FUNC_MSG_IN;
	return ret;
}
int setQOSEgressControl(void *cdata, int len)
{
	int ports,rate,i;
	FUNC_MSG_IN;
	if (sizeof(struct PortmapSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}
	ports = ((struct PortmapSetting *)cdata) ->portmap;
	rate = ((struct PortmapSetting *)cdata) ->pmdata;
	if (ports & ~0x1FFFFFFF)
	{
		ip1829drv_dbg("Error: portmap=%08X\n", ports);
		return -EINVAL;
	}
	if (rate & ~0x00003FFF)
	{
		ip1829drv_dbg("Error: pmdata=%08X\n", rate);
		return -EINVAL;
	}
	IP2Page(8);
	for(i=0;i<SWITCH_MAX_IPORT_CNT;i++)
	{
		if((ports>>i)&0x1)
		{
			Write_Reg(P8REG_EGRESS_RATE_CTRL0+i,(u16)rate);
		}
	}
	ip1829drv_dbg("cdata ->portmap=0x%08X\n", ports);
	ip1829drv_dbg("cdata ->pmdata=0x%08X\n", rate);
	FUNC_MSG_OUT;
	return 0;
}
int getQOSEgressControl(void *cdata, int len)
{
	int ports,i;
	FUNC_MSG_IN;
	if (sizeof(struct PortmapSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}
	ports = ((struct PortmapSetting *)cdata) ->portmap;
	if (ports & ~0x1FFFFFFF)
	{
		ip1829drv_dbg("Error: portmap=%08X\n", ports);
		return -EINVAL;
	}
	IP2Page(8);
	for(i=0;i<SWITCH_MAX_IPORT_CNT;i++)
	{
		if((ports>>i)&0x1)
		{
			Write_Reg(P8REG_EGRESS_RATE_CTRL0+i,0x8000);
			((struct PortmapSetting *)cdata)->pmdata = (int)(Read_Reg(P8REG_EGRESS_RATE_CTRL0+i)&0x3FFF);
			break;
		}
	}
	ip1829drv_dbg("cdata ->portmap=0x%08X\n", (u16)((struct PortmapSetting *)cdata) ->portmap);
	ip1829drv_dbg("cdata ->pmdata=0x%08X\n", ((struct PortmapSetting *)cdata) ->pmdata);
	FUNC_MSG_OUT;
	return 0;
}

int setQOSRemap(void *cdata, int len)
{
	int port;
	u8 queue,remap;
	FUNC_MSG_IN;
	if (sizeof(struct qos_remap) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}
	port = ((struct qos_remap *)cdata) ->port;
	queue = ((struct qos_remap *)cdata) ->queue;
	remap = ((struct qos_remap *)cdata) ->remap;

	if(port<1 || port>SWITCH_MAX_IPORT_CNT)
		return -EINVAL;
	if(queue>OP_QOS_REMAP_TX_Q7)
	{
		ip1829drv_dbg("Error: queue=%d\n", queue);
		return -EINVAL;
	}
	if(remap>7)
	{
		ip1829drv_dbg("Error: remap=%d\n", remap);
		return -EINVAL;
	}
	ip1829drv_dbg("cdata ->port=0x%d\n", ((struct qos_remap *)cdata) ->port);
	ip1829drv_dbg("cdata ->queue=0x%d\n", ((struct qos_remap *)cdata) ->queue);
	ip1829drv_dbg("cdata ->remap=0x%d\n", ((struct qos_remap *)cdata) ->remap);
	port-=1;
	if(queue<=OP_QOS_REMAP_RX_Q7)//RX
	{
		_WriteRegBits(6, P6REG_QOS_REMAP_RX0+(port*2)+(queue/5), (queue%5)*3, 3, remap);
	}
	else//TX
	{
		queue-=8;
		_WriteRegBits(8, P8REG_QOS_REMAP_TX0+(port*2)+(queue/5), (queue%5)*3, 3, remap);
	}
	FUNC_MSG_OUT;
	return 0;
}
int getQOSRemap(void *cdata, int len)
{
	int port;
	u8 queue;
	u16 u16dat;
	FUNC_MSG_IN;
	if (sizeof(struct qos_remap) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}
	port = ((struct qos_remap *)cdata) ->port;
	queue = ((struct qos_remap *)cdata) ->queue;
	if(port<1 || port>SWITCH_MAX_IPORT_CNT)
		return -EINVAL;
	if(queue>OP_QOS_REMAP_TX_Q7)
	{
		ip1829drv_dbg("Error: queue=%d\n", queue);
		return -EINVAL;
	}
	ip1829drv_dbg("cdata ->port=0x%d\n", ((struct qos_remap *)cdata) ->port);
	ip1829drv_dbg("cdata ->queue=0x%d\n", ((struct qos_remap *)cdata) ->queue);
	port-=1;
	if(queue<=OP_QOS_REMAP_RX_Q7)//RX
	{
		IP2Page(6);
		u16dat = Read_Reg(P6REG_QOS_REMAP_RX0+(port*2)+(queue/5));
	}
	else//TX
	{
		queue-=8;
		IP2Page(8);
		u16dat = Read_Reg(P8REG_QOS_REMAP_TX0+(port*2)+(queue/5));
	}
	((struct qos_remap *)cdata) ->remap = (u8)((u16dat>>((queue%5)*3))&0x7);

	ip1829drv_dbg("cdata ->remap=0x%d\n", ((struct qos_remap *)cdata) ->remap);
	FUNC_MSG_OUT;
	return 0;
}
//------------ QOS functions:ip1829 end--------------------------
//------------ ACL functions:common ----------------------------
struct acl_man	*m_acl;	// acl golbal manage variable
int acl_write_table_0010(int index, int link, struct acl_rule *r);
int acl_write_table_0011(int index, int link, struct acl_rule *r);
int acl_write_table_0100(int index, int link, struct acl_rule *r);
int acl_write_table_0101(int index, int link, struct acl_rule *r);
int acl_write_table_0110(int index, int link, struct acl_rule *r);
int acl_write_table_0111(int index, int link, struct acl_rule *r);
int acl_write_table_1000(int index, int link, struct acl_rule *r);
int acl_write_table_1001(int index, int link, struct acl_rule *r);
int acl_write_table_1010(int index, int link, struct acl_rule *r);
int acl_write_table_1011(int index, int link, struct acl_rule *r);
int acl_write_table_1100(int index, int link, struct acl_rule *r);
int acl_write_table_1110(int index, int link, struct acl_rule *r);
int acl_write_table_1111(int index, int link, struct acl_rule *r);
int (*acl_write_table_rule[])(int index, int link, struct acl_rule *r) = {
	NULL,
	acl_write_table_0010,
	acl_write_table_0011,
	acl_write_table_0100,
	acl_write_table_0101,
	acl_write_table_0110,
	acl_write_table_0111,
	acl_write_table_1000,
	acl_write_table_1001,
	acl_write_table_1010,
	acl_write_table_1011,
	acl_write_table_1100,
	NULL,
	acl_write_table_1110,
	acl_write_table_1111
};
int acl_write_table_drop(int index, struct acl_rule *r);
int acl_write_table_a1(int index, struct acl_rule *r);
int acl_write_table_a2(int index, struct acl_rule *r);
int acl_write_table_a3(int index, struct acl_rule *r);
int acl_write_table_a4(int index, struct acl_rule *r);
int acl_write_table_a5(int index, struct acl_rule *r);
int (*acl_write_table_act[])(int index, struct acl_rule *r) = {
	acl_write_table_drop,
	acl_write_table_a1,
	acl_write_table_a2,
	acl_write_table_a3,
	acl_write_table_a4,
	acl_write_table_a5
};

void acl_init(void)
{
	int i;
	ip1829drv_dbg("<DBG_DR_ACL> acl_init()\n");

	m_acl = kmalloc(sizeof(struct acl_man), GFP_KERNEL);
	if(m_acl == NULL)
	{
		ip1829drv_dbg("Error: acl_init() acl_man malloc failed.\n");
		return;
	}

	INIT_LIST_HEAD(&m_acl ->rule_list);
	m_acl ->num_used_rules = 0;
	m_acl ->num_used_entries = 0;
	for(i=0; i<8; i++)
	{
		m_acl ->used_entry_mask[i] = 0;
	}
}

int acl_find_index(int num)
{
	int i=0, j=0;
	unsigned char m = 0;
	unsigned long t = 0;

	for(i=0; i<num; i++)
	{
		m |= BIT(i);
	}

	for(i=0; i<8; i++)
	{
		if(i != 7)
		{
			t = (unsigned long)m_acl ->used_entry_mask[i] | ((unsigned long)m_acl ->used_entry_mask[i+1]<<16);
		}
		else
		{
			t = (unsigned long)m_acl ->used_entry_mask[i] | 0xFFFF0000;
		}

		for(j=0; j<16; j++)
		{
			if(!((t>>j) & m))
			{
				return (i*16+j);
			}
		}
	}

	return -ENOMEM;
}

int acl_clean_table(int index)
{
	IP2Page(2);

	Write_Reg(P2REG_ACL_TABLE_DATA_E4, 0);
	Write_Reg(P2REG_ACL_TABLE_DATA_E5, 0);
	Write_Reg(P2REG_ACL_TABLE_DATA_E6, 0);
	Write_Reg(P2REG_ACL_TABLE_DATA_E7, 0);
	Write_Reg(P2REG_ACL_TABLE_DATA_E8, 0);
	Write_Reg(P2REG_ACL_TABLE_DATA_E9, 0);
	Write_Reg(P2REG_ACL_TABLE_DATA_EA, 0);
	Write_Reg(P2REG_ACL_TABLE_DATA_EB, 0);
	Write_Reg(P2REG_ACL_TABLE_DATA_EC, 0);
	Write_Reg(P2REG_ACL_TABLE_DATA_ED, 0);

	Write_Reg(P2REG_ACL_TABLE_ACCESS, 0xC000|index);
	return 0;
}

int acl_write_table_0010(int index, int link, struct acl_rule *r)
{
	u16 tmp;

	IP2Page(2);
	//E4 & E5
	if(r->rule_valid & ACL_RULE_VALID_DP_R)
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E4, r->dp_lo);
		Write_Reg(P2REG_ACL_TABLE_DATA_E5, r->dp_hi);
	}
	else
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E4, 0);
		Write_Reg(P2REG_ACL_TABLE_DATA_E5, 0xFFFF);
	}
	//E6 & E7
	if(r->rule_valid & ACL_RULE_VALID_SP_R)
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E6, r->sp_lo);
		Write_Reg(P2REG_ACL_TABLE_DATA_E7, r->sp_hi);
	}
	else
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E6, 0);
		Write_Reg(P2REG_ACL_TABLE_DATA_E7, 0xFFFF);
	}
	//E8 & E9
	tmp = 0;
	if(r->rule_valid & ACL_RULE_VALID_DIP)
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E8, r->dip4_addr&0xFFFF);
		Write_Reg(P2REG_ACL_TABLE_DATA_E9, (r->dip4_addr>>16)&0xFFFF);
		tmp |= ((u16)(r->dip_mask&0x7)<<5);
	}
	else
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E8, 0);
		Write_Reg(P2REG_ACL_TABLE_DATA_E9, 0);
		tmp |= (0x1<<5);		// any dip
	}
	//EA & EB
	if(r->rule_valid & ACL_RULE_VALID_SIP)
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_EA, r->sip4_addr&0xFFFF);
		Write_Reg(P2REG_ACL_TABLE_DATA_EB, (r->sip4_addr>>16)&0xFFFF);
		tmp |= ((u16)(r->sip_mask&0x7)<<8);
	}
	else
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_EA, 0);
		Write_Reg(P2REG_ACL_TABLE_DATA_EB, 0);
		tmp |= (0x1<<8);		// any sip
	}
	//EC
	if(r->rule_valid & ACL_RULE_VALID_INGRESS_PORT)
	{
		tmp |= r->ingress_port;
	}
	Write_Reg(P2REG_ACL_TABLE_DATA_EC, tmp);
	//ED
	tmp = ((ACL_SELECT_MODE_0010<<3) | (link<<1));
	Write_Reg(P2REG_ACL_TABLE_DATA_ED, tmp);

	Write_Reg(P2REG_ACL_TABLE_ACCESS, 0xC000|index);
	return 0;
}

int acl_write_table_0011(int index, int link, struct acl_rule *r)
{
	u16 tmp;

	IP2Page(2);
	//E6
	tmp = 0;
	if(r->rule_valid & ACL_RULE_VALID_DP)
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E6, r->dp_hi);
	}
	else
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E6, 0);
		tmp |= BIT(0);
	}
	//E7
	if(r->rule_valid & ACL_RULE_VALID_SP)
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E7, r->sp_hi);
	}
	else
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E7, 0);
		tmp |= BIT(1);
	}
	//E4
	Write_Reg(P2REG_ACL_TABLE_DATA_E4, tmp);
	//E5
	tmp = 0;
	if(r->rule_valid & ACL_RULE_VALID_TCP_FLAG)
	{
		tmp = (((u16)r->tcp_flag&0xFF)<<8) | ((u16)r->tcp_flag_mask&0xFF);
	}
	Write_Reg(P2REG_ACL_TABLE_DATA_E5, tmp);
	//E8 & E9
	tmp = 0;
	if(r->rule_valid & ACL_RULE_VALID_DIP)
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E8, r->dip4_addr&0xFFFF);
		Write_Reg(P2REG_ACL_TABLE_DATA_E9, (r->dip4_addr>>16)&0xFFFF);
		tmp |= ((u16)(r->dip_mask&0x7)<<5);
	}
	else
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E8, 0);
		Write_Reg(P2REG_ACL_TABLE_DATA_E9, 0);
		tmp |= (0x1<<5);		// any dip
	}
	//EA & EB
	if(r->rule_valid & ACL_RULE_VALID_SIP)
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_EA, r->sip4_addr&0xFFFF);
		Write_Reg(P2REG_ACL_TABLE_DATA_EB, (r->sip4_addr>>16)&0xFFFF);
		tmp |= ((u16)(r->sip_mask&0x7)<<8);
	}
	else
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_EA, 0);
		Write_Reg(P2REG_ACL_TABLE_DATA_EB, 0);
		tmp |= (0x1<<8);		// any sip
	}
	//EC
	if(r->rule_valid & ACL_RULE_VALID_INGRESS_PORT)
	{
		tmp |= r->ingress_port;
	}
	Write_Reg(P2REG_ACL_TABLE_DATA_EC, tmp);
	//ED
	tmp = ((ACL_SELECT_MODE_0011<<3) | (link<<1));
	Write_Reg(P2REG_ACL_TABLE_DATA_ED, tmp);

	Write_Reg(P2REG_ACL_TABLE_ACCESS, 0xC000|index);
	return 0;
}

int acl_write_table_0100(int index, int link, struct acl_rule *r)
{
	u16 tmp;

	IP2Page(2);
	//E4 & E5
	tmp = 0;
	if(r->rule_valid & ACL_RULE_VALID_SIP)
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E4, r->sip4_addr&0xFFFF);
		Write_Reg(P2REG_ACL_TABLE_DATA_E5, (r->sip4_addr>>16)&0xFFFF);
		tmp |= ((u16)(r->sip_mask&0x7)<<5);
	}
	else
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E4, 0);
		Write_Reg(P2REG_ACL_TABLE_DATA_E5, 0);
		tmp |= (0x1<<5);		// any sip
	}
	//E6 ~ E8
	if(r->rule_valid & ACL_RULE_VALID_SMAC)
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E6, ((u16)(r->smac[4]&0xFF)<<8) | ((u16)(r->smac[5]&0xFF)));
		Write_Reg(P2REG_ACL_TABLE_DATA_E7, ((u16)(r->smac[2]&0xFF)<<8) | ((u16)(r->smac[3]&0xFF)));
		Write_Reg(P2REG_ACL_TABLE_DATA_E8, ((u16)(r->smac[0]&0xFF)<<8) | ((u16)(r->smac[1]&0xFF)));
		tmp |= ((u16)(r->smac_mask&0x3)<<8);
	}
	else
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E6, 0);
		Write_Reg(P2REG_ACL_TABLE_DATA_E7, 0);
		Write_Reg(P2REG_ACL_TABLE_DATA_E8, 0);
		tmp |= (0x1<<8);		// any smac
	}
	//E9 ~ EB
	if(r->rule_valid & ACL_RULE_VALID_DMAC)
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E9, ((u16)(r->dmac[4]&0xFF)<<8) | ((u16)(r->dmac[5]&0xFF)));
		Write_Reg(P2REG_ACL_TABLE_DATA_EA, ((u16)(r->dmac[2]&0xFF)<<8) | ((u16)(r->dmac[3]&0xFF)));
		Write_Reg(P2REG_ACL_TABLE_DATA_EB, ((u16)(r->dmac[0]&0xFF)<<8) | ((u16)(r->dmac[1]&0xFF)));
		tmp |= ((u16)(r->dmac_mask&0x3)<<10);
	}
	else
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E9, 0);
		Write_Reg(P2REG_ACL_TABLE_DATA_EA, 0);
		Write_Reg(P2REG_ACL_TABLE_DATA_EB, 0);
		tmp |= (0x1<<10);		// any dmac
	}
	//EC
	if(r->rule_valid & ACL_RULE_VALID_INGRESS_PORT)
	{
		tmp |= r->ingress_port;
	}
	Write_Reg(P2REG_ACL_TABLE_DATA_EC, tmp);
	//ED
	tmp = ((ACL_SELECT_MODE_0100<<3) | (link<<1));
	Write_Reg(P2REG_ACL_TABLE_DATA_ED, tmp);

	Write_Reg(P2REG_ACL_TABLE_ACCESS, 0xC000|index);
	return 0;
}

int acl_write_table_0101(int index, int link, struct acl_rule *r)
{
	u16 tmp;

	IP2Page(2);
	//E4 & E5
	tmp = 0;
	if(r->rule_valid & ACL_RULE_VALID_DIP)
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E4, r->dip4_addr&0xFFFF);
		Write_Reg(P2REG_ACL_TABLE_DATA_E5, (r->dip4_addr>>16)&0xFFFF);
		tmp |= ((u16)(r->dip_mask&0x7)<<5);
	}
	else
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E4, 0);
		Write_Reg(P2REG_ACL_TABLE_DATA_E5, 0);
		tmp |= (0x1<<5);		// any dip
	}
	//E6 ~ E8
	if(r->rule_valid & ACL_RULE_VALID_SMAC)
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E6, ((u16)(r->smac[4]&0xFF)<<8) | ((u16)(r->smac[5]&0xFF)));
		Write_Reg(P2REG_ACL_TABLE_DATA_E7, ((u16)(r->smac[2]&0xFF)<<8) | ((u16)(r->smac[3]&0xFF)));
		Write_Reg(P2REG_ACL_TABLE_DATA_E8, ((u16)(r->smac[0]&0xFF)<<8) | ((u16)(r->smac[1]&0xFF)));
		tmp |= ((u16)(r->smac_mask&0x3)<<8);
	}
	else
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E6, 0);
		Write_Reg(P2REG_ACL_TABLE_DATA_E7, 0);
		Write_Reg(P2REG_ACL_TABLE_DATA_E8, 0);
		tmp |= (0x1<<8);		// any smac
	}
	//E9 ~ EB
	if(r->rule_valid & ACL_RULE_VALID_DMAC)
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E9, ((u16)(r->dmac[4]&0xFF)<<8) | ((u16)(r->dmac[5]&0xFF)));
		Write_Reg(P2REG_ACL_TABLE_DATA_EA, ((u16)(r->dmac[2]&0xFF)<<8) | ((u16)(r->dmac[3]&0xFF)));
		Write_Reg(P2REG_ACL_TABLE_DATA_EB, ((u16)(r->dmac[0]&0xFF)<<8) | ((u16)(r->dmac[1]&0xFF)));
		tmp |= ((u16)(r->dmac_mask&0x3)<<10);
	}
	else
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E9, 0);
		Write_Reg(P2REG_ACL_TABLE_DATA_EA, 0);
		Write_Reg(P2REG_ACL_TABLE_DATA_EB, 0);
		tmp |= (0x1<<10);		// any dmac
	}
	//EC
	if(r->rule_valid & ACL_RULE_VALID_INGRESS_PORT)
	{
		tmp |= r->ingress_port;
	}
	Write_Reg(P2REG_ACL_TABLE_DATA_EC, tmp);
	//ED
	tmp = ((ACL_SELECT_MODE_0101<<3) | (link<<1));
	Write_Reg(P2REG_ACL_TABLE_DATA_ED, tmp);

	Write_Reg(P2REG_ACL_TABLE_ACCESS, 0xC000|index);
	return 0;
}

int acl_write_table_0110(int index, int link, struct acl_rule *r)
{
	u16 tmp;

	IP2Page(2);
	//E4
	tmp = 0;
	if(r->rule_valid & ACL_RULE_VALID_ETH_TYPE)
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E4, r->eth_type);
	}
	else
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E4, 0);
		tmp |= BIT(6);
	}
	//E5
	if(r->rule_valid & ACL_RULE_VALID_VLAN)
	{
		if(r->rule_valid & ACL_RULE_VALID_COS)
		{
			Write_Reg(P2REG_ACL_TABLE_DATA_E5, (r->vlan | (r->cos<<12)));
		}
		else
		{
			tmp |= BIT(7);
			Write_Reg(P2REG_ACL_TABLE_DATA_E5, r->vlan);
		}
	}
	else
	{
		tmp |= BIT(7);
		Write_Reg(P2REG_ACL_TABLE_DATA_E5, 0x8000);
	}
	//E6 ~ E8
	if(r->rule_valid & ACL_RULE_VALID_SMAC)
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E6, ((u16)(r->smac[4]&0xFF)<<8) | ((u16)(r->smac[5]&0xFF)));
		Write_Reg(P2REG_ACL_TABLE_DATA_E7, ((u16)(r->smac[2]&0xFF)<<8) | ((u16)(r->smac[3]&0xFF)));
		Write_Reg(P2REG_ACL_TABLE_DATA_E8, ((u16)(r->smac[0]&0xFF)<<8) | ((u16)(r->smac[1]&0xFF)));
		tmp |= ((u16)(r->smac_mask&0x3)<<8);
	}
	else
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E6, 0);
		Write_Reg(P2REG_ACL_TABLE_DATA_E7, 0);
		Write_Reg(P2REG_ACL_TABLE_DATA_E8, 0);
		tmp |= (0x1<<8);		// any smac
	}
	//E9 ~ EB
	if(r->rule_valid & ACL_RULE_VALID_DMAC)
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E9, ((u16)(r->dmac[4]&0xFF)<<8) | ((u16)(r->dmac[5]&0xFF)));
		Write_Reg(P2REG_ACL_TABLE_DATA_EA, ((u16)(r->dmac[2]&0xFF)<<8) | ((u16)(r->dmac[3]&0xFF)));
		Write_Reg(P2REG_ACL_TABLE_DATA_EB, ((u16)(r->dmac[0]&0xFF)<<8) | ((u16)(r->dmac[1]&0xFF)));
		tmp |= ((u16)(r->dmac_mask&0x3)<<10);
	}
	else
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E9, 0);
		Write_Reg(P2REG_ACL_TABLE_DATA_EA, 0);
		Write_Reg(P2REG_ACL_TABLE_DATA_EB, 0);
		tmp |= (0x1<<10);		// any dmac
	}
	//EC
	if(r->rule_valid & ACL_RULE_VALID_INGRESS_PORT)
	{
		tmp |= r->ingress_port;
	}
	Write_Reg(P2REG_ACL_TABLE_DATA_EC, tmp);
	//ED
	tmp = ((ACL_SELECT_MODE_0110<<3) | (link<<1));
	Write_Reg(P2REG_ACL_TABLE_DATA_ED, tmp);

	Write_Reg(P2REG_ACL_TABLE_ACCESS, 0xC000|index);
	return 0;
}

int acl_write_table_0111(int index, int link, struct acl_rule *r)
{
	u16 tmp;

	IP2Page(2);
	//E4 no ctag rule
	tmp = 0;
	Write_Reg(P2REG_ACL_TABLE_DATA_E4, 0);
	tmp |= BIT(6);
	//E5 no stag rule
	Write_Reg(P2REG_ACL_TABLE_DATA_E5, 0);
	tmp |= BIT(7);
	//E6 ~ E8
	if(r->rule_valid & ACL_RULE_VALID_SMAC)
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E6, ((u16)(r->smac[4]&0xFF)<<8) | ((u16)(r->smac[5]&0xFF)));
		Write_Reg(P2REG_ACL_TABLE_DATA_E7, ((u16)(r->smac[2]&0xFF)<<8) | ((u16)(r->smac[3]&0xFF)));
		Write_Reg(P2REG_ACL_TABLE_DATA_E8, ((u16)(r->smac[0]&0xFF)<<8) | ((u16)(r->smac[1]&0xFF)));
		tmp |= ((u16)(r->smac_mask&0x3)<<8);
	}
	else
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E6, 0);
		Write_Reg(P2REG_ACL_TABLE_DATA_E7, 0);
		Write_Reg(P2REG_ACL_TABLE_DATA_E8, 0);
		tmp |= (0x1<<8);		// any smac
	}
	//E9 ~ EB
	if(r->rule_valid & ACL_RULE_VALID_DMAC)
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E9, ((u16)(r->dmac[4]&0xFF)<<8) | ((u16)(r->dmac[5]&0xFF)));
		Write_Reg(P2REG_ACL_TABLE_DATA_EA, ((u16)(r->dmac[2]&0xFF)<<8) | ((u16)(r->dmac[3]&0xFF)));
		Write_Reg(P2REG_ACL_TABLE_DATA_EB, ((u16)(r->dmac[0]&0xFF)<<8) | ((u16)(r->dmac[1]&0xFF)));
		tmp |= ((u16)(r->dmac_mask&0x3)<<10);
	}
	else
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E9, 0);
		Write_Reg(P2REG_ACL_TABLE_DATA_EA, 0);
		Write_Reg(P2REG_ACL_TABLE_DATA_EB, 0);
		tmp |= (0x1<<10);		// any dmac
	}
	//EC
	if(r->rule_valid & ACL_RULE_VALID_INGRESS_PORT)
	{
		tmp |= r->ingress_port;
	}
	Write_Reg(P2REG_ACL_TABLE_DATA_EC, tmp);
	//ED
	tmp = ((ACL_SELECT_MODE_0111<<3) | (link<<1));
	Write_Reg(P2REG_ACL_TABLE_DATA_ED, tmp);

	Write_Reg(P2REG_ACL_TABLE_ACCESS, 0xC000|index);
	return 0;
}

int acl_write_table_1000(int index, int link, struct acl_rule *r)
{
	u16 tmp=0, tmp2=0;

	IP2Page(2);
	//E4
	//route
	if(r->rule_valid & ACL_RULE_VALID_ROUTE_PORT)
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E4, (r->route_port&0x1f)<< 8);
	}
	else
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E4, 0x8000);
	}
	//E5
	if(r->rule_valid & ACL_RULE_VALID_VLAN)
	{
		if(r->rule_valid & ACL_RULE_VALID_COS)
		{
			Write_Reg(P2REG_ACL_TABLE_DATA_E5, (r->vlan | (r->cos<<12)));
		}
		else
		{
			tmp |= BIT(8);
			Write_Reg(P2REG_ACL_TABLE_DATA_E5, r->vlan);
		}
	}
	else
	{
		tmp |= BIT(8);
		Write_Reg(P2REG_ACL_TABLE_DATA_E5, 0x8000);
	}
	//E6
	// ip_proto
	if(r->rule_valid & ACL_RULE_VALID_IP_PROT)
	{
		tmp2 |= ((u16)r->ip_prot&0xFF);
	}
	else
	{
		tmp |= BIT(9);		// any ip_prot
	}
	// dscp
	if(r->rule_valid & ACL_RULE_VALID_DSCP)
	{
		tmp2 |= ((u16)(r->r_dscp&0xFF)<<8);
	}
	else
	{
		tmp2 |= 0x8000;
	}
	Write_Reg(P2REG_ACL_TABLE_DATA_E6, tmp2);
	//E7 & E8
	if(r->rule_valid & ACL_RULE_VALID_SIP)
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E7, r->sip4_addr&0xFFFF);
		Write_Reg(P2REG_ACL_TABLE_DATA_E8, (r->sip4_addr>>16)&0xFFFF);
		tmp |= ((u16)(r->sip_mask&0x7)<<5);
	}
	else
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E7, 0);
		Write_Reg(P2REG_ACL_TABLE_DATA_E8, 0);
		tmp |= (0x1<<5);		// any sip
	}
	//E9 ~ EB
	if(r->rule_valid & ACL_RULE_VALID_SMAC)
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E9, ((u16)(r->smac[4]&0xFF)<<8) | ((u16)(r->smac[5]&0xFF)));
		Write_Reg(P2REG_ACL_TABLE_DATA_EA, ((u16)(r->smac[2]&0xFF)<<8) | ((u16)(r->smac[3]&0xFF)));
		Write_Reg(P2REG_ACL_TABLE_DATA_EB, ((u16)(r->smac[0]&0xFF)<<8) | ((u16)(r->smac[1]&0xFF)));
		tmp |= ((u16)(r->smac_mask&0x3)<<10);
	}
	else
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E9, 0);
		Write_Reg(P2REG_ACL_TABLE_DATA_EA, 0);
		Write_Reg(P2REG_ACL_TABLE_DATA_EB, 0);
		tmp |= (0x1<<10);		// any smac
	}
	//EC
	if(r->rule_valid & ACL_RULE_VALID_INGRESS_PORT)
	{
		tmp |= r->ingress_port;
	}
	Write_Reg(P2REG_ACL_TABLE_DATA_EC, tmp);
	//ED
	tmp = ((ACL_SELECT_MODE_1000<<3) | (link<<1));
	Write_Reg(P2REG_ACL_TABLE_DATA_ED, tmp);

	Write_Reg(P2REG_ACL_TABLE_ACCESS, 0xC000|index);
	return 0;
}

int acl_write_table_1001(int index, int link, struct acl_rule *r)
{
	u16 tmp=0, tmp2=0;

	IP2Page(2);
	//E4
	//route
	if(r->rule_valid & ACL_RULE_VALID_ROUTE_PORT)
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E4, (r->route_port&0x1f)<< 8);
	}
	else
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E4, 0x8000);
	}
	//E5
	if(r->rule_valid & ACL_RULE_VALID_VLAN)
	{
		if(r->rule_valid & ACL_RULE_VALID_COS)
		{
			Write_Reg(P2REG_ACL_TABLE_DATA_E5, (r->vlan | (r->cos<<12)));
		}
		else
		{
			tmp |= BIT(8);
			Write_Reg(P2REG_ACL_TABLE_DATA_E5, r->vlan);
		}
	}
	else
	{
		tmp |= BIT(8);
		Write_Reg(P2REG_ACL_TABLE_DATA_E5, 0x8000);
	}
	//E6
	// ip_proto
	if(r->rule_valid & ACL_RULE_VALID_IP_PROT)
	{
		tmp2 |= ((u16)r->ip_prot&0xFF);
	}
	else
	{
		tmp |= BIT(9);		// any ip_prot
	}
	// dscp
	if(r->rule_valid & ACL_RULE_VALID_DSCP)
	{
		tmp2 |= ((u16)(r->r_dscp&0xFF)<<8);
	}
	else
	{
		tmp2 |= 0x8000;
	}
	Write_Reg(P2REG_ACL_TABLE_DATA_E6, tmp2);
	//E7 & E8
	if(r->rule_valid & ACL_RULE_VALID_DIP)
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E7, r->dip4_addr&0xFFFF);
		Write_Reg(P2REG_ACL_TABLE_DATA_E8, (r->dip4_addr>>16)&0xFFFF);
		tmp |= ((u16)(r->dip_mask&0x7)<<5);
	}
	else
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E7, 0);
		Write_Reg(P2REG_ACL_TABLE_DATA_E8, 0);
		tmp |= (0x1<<5);		// any dip
	}
	//E9 ~ EB
	if(r->rule_valid & ACL_RULE_VALID_SMAC)
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E9, ((u16)(r->smac[4]&0xFF)<<8) | ((u16)(r->smac[5]&0xFF)));
		Write_Reg(P2REG_ACL_TABLE_DATA_EA, ((u16)(r->smac[2]&0xFF)<<8) | ((u16)(r->smac[3]&0xFF)));
		Write_Reg(P2REG_ACL_TABLE_DATA_EB, ((u16)(r->smac[0]&0xFF)<<8) | ((u16)(r->smac[1]&0xFF)));
		tmp |= ((u16)(r->smac_mask&0x3)<<10);
	}
	else
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E9, 0);
		Write_Reg(P2REG_ACL_TABLE_DATA_EA, 0);
		Write_Reg(P2REG_ACL_TABLE_DATA_EB, 0);
		tmp |= (0x1<<10);		// any smac
	}
	//EC
	if(r->rule_valid & ACL_RULE_VALID_INGRESS_PORT)
	{
		tmp |= r->ingress_port;
	}
	Write_Reg(P2REG_ACL_TABLE_DATA_EC, tmp);
	//ED
	tmp = ((ACL_SELECT_MODE_1001<<3) | (link<<1));
	Write_Reg(P2REG_ACL_TABLE_DATA_ED, tmp);

	Write_Reg(P2REG_ACL_TABLE_ACCESS, 0xC000|index);
	return 0;
}

int acl_write_table_1010(int index, int link, struct acl_rule *r)
{
	u16 tmp=0, tmp2=0;

	IP2Page(2);
	//E4
	//route
	if(r->rule_valid & ACL_RULE_VALID_ROUTE_PORT)
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E4, (r->route_port&0x1f)<< 8);
	}
	else
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E4, 0x8000);
	}
	//E5
	if(r->rule_valid & ACL_RULE_VALID_VLAN)
	{
		if(r->rule_valid & ACL_RULE_VALID_COS)
		{
			Write_Reg(P2REG_ACL_TABLE_DATA_E5, (r->vlan | (r->cos<<12)));
		}
		else
		{
			tmp |= BIT(8);
			Write_Reg(P2REG_ACL_TABLE_DATA_E5, r->vlan);
		}
	}
	else
	{
		tmp |= BIT(8);
		Write_Reg(P2REG_ACL_TABLE_DATA_E5, 0x8000);
	}
	//E6
	// ip_proto
	if(r->rule_valid & ACL_RULE_VALID_IP_PROT)
	{
		tmp2 |= ((u16)r->ip_prot&0xFF);
	}
	else
	{
		tmp |= BIT(9);		// any ip_prot
	}
	// dscp
	if(r->rule_valid & ACL_RULE_VALID_DSCP)
	{
		tmp2 |= ((u16)(r->r_dscp&0xFF)<<8);
	}
	else
	{
		tmp2 |= 0x8000;
	}
	Write_Reg(P2REG_ACL_TABLE_DATA_E6, tmp2);
	//E7 & E8
	if(r->rule_valid & ACL_RULE_VALID_SIP)
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E7, r->sip4_addr&0xFFFF);
		Write_Reg(P2REG_ACL_TABLE_DATA_E8, (r->sip4_addr>>16)&0xFFFF);
		tmp |= ((u16)(r->sip_mask&0x7)<<5);
	}
	else
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E7, 0);
		Write_Reg(P2REG_ACL_TABLE_DATA_E8, 0);
		tmp |= (0x1<<5);		// any sip
	}
	//E9 ~ EB
	if(r->rule_valid & ACL_RULE_VALID_DMAC)
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E9, ((u16)(r->dmac[4]&0xFF)<<8) | ((u16)(r->dmac[5]&0xFF)));
		Write_Reg(P2REG_ACL_TABLE_DATA_EA, ((u16)(r->dmac[2]&0xFF)<<8) | ((u16)(r->dmac[3]&0xFF)));
		Write_Reg(P2REG_ACL_TABLE_DATA_EB, ((u16)(r->dmac[0]&0xFF)<<8) | ((u16)(r->dmac[1]&0xFF)));
		tmp |= ((u16)(r->dmac_mask&0x3)<<10);
	}
	else
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E9, 0);
		Write_Reg(P2REG_ACL_TABLE_DATA_EA, 0);
		Write_Reg(P2REG_ACL_TABLE_DATA_EB, 0);
		tmp |= (0x1<<10);		// any dmac
	}
	//EC
	if(r->rule_valid & ACL_RULE_VALID_INGRESS_PORT)
	{
		tmp |= r->ingress_port;
	}
	Write_Reg(P2REG_ACL_TABLE_DATA_EC, tmp);
	//ED
	tmp = ((ACL_SELECT_MODE_1010<<3) | (link<<1));
	Write_Reg(P2REG_ACL_TABLE_DATA_ED, tmp);

	Write_Reg(P2REG_ACL_TABLE_ACCESS, 0xC000|index);
	return 0;
}

int acl_write_table_1011(int index, int link, struct acl_rule *r)
{
	u16 tmp=0, tmp2=0;

	IP2Page(2);
	//E4
	//route
	if(r->rule_valid & ACL_RULE_VALID_ROUTE_PORT)
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E4, (r->route_port&0x1f)<< 8);
	}
	else
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E4, 0x8000);
	}
	//E5
	if(r->rule_valid & ACL_RULE_VALID_VLAN)
	{
		if(r->rule_valid & ACL_RULE_VALID_COS)
		{
			Write_Reg(P2REG_ACL_TABLE_DATA_E5, (r->vlan | (r->cos<<12)));
		}
		else
		{
			tmp |= BIT(8);
			Write_Reg(P2REG_ACL_TABLE_DATA_E5, r->vlan);
		}
	}
	else
	{
		tmp |= BIT(8);
		Write_Reg(P2REG_ACL_TABLE_DATA_E5, 0x8000);
	}
	//E6
	// ip_proto
	if(r->rule_valid & ACL_RULE_VALID_IP_PROT)
	{
		tmp2 |= ((u16)r->ip_prot&0xFF);
	}
	else
	{
		tmp |= BIT(9);		// any ip_prot
	}
	// dscp
	if(r->rule_valid & ACL_RULE_VALID_DSCP)
	{
		tmp2 |= ((u16)(r->r_dscp&0xFF)<<8);
	}
	else
	{
		tmp2 |= 0x8000;
	}
	Write_Reg(P2REG_ACL_TABLE_DATA_E6, tmp2);
	//E7 & E8
	if(r->rule_valid & ACL_RULE_VALID_DIP)
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E7, r->dip4_addr&0xFFFF);
		Write_Reg(P2REG_ACL_TABLE_DATA_E8, (r->dip4_addr>>16)&0xFFFF);
		tmp |= ((u16)(r->dip_mask&0x7)<<5);
	}
	else
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E7, 0);
		Write_Reg(P2REG_ACL_TABLE_DATA_E8, 0);
		tmp |= (0x1<<5);		// any dip
	}
	//E9 ~ EB
	if(r->rule_valid & ACL_RULE_VALID_DMAC)
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E9, ((u16)(r->dmac[4]&0xFF)<<8) | ((u16)(r->dmac[5]&0xFF)));
		Write_Reg(P2REG_ACL_TABLE_DATA_EA, ((u16)(r->dmac[2]&0xFF)<<8) | ((u16)(r->dmac[3]&0xFF)));
		Write_Reg(P2REG_ACL_TABLE_DATA_EB, ((u16)(r->dmac[0]&0xFF)<<8) | ((u16)(r->dmac[1]&0xFF)));
		tmp |= ((u16)(r->dmac_mask&0x3)<<10);
	}
	else
	{
		Write_Reg(P2REG_ACL_TABLE_DATA_E9, 0);
		Write_Reg(P2REG_ACL_TABLE_DATA_EA, 0);
		Write_Reg(P2REG_ACL_TABLE_DATA_EB, 0);
		tmp |= (0x1<<10);		// any dmac
	}
	//EC
	if(r->rule_valid & ACL_RULE_VALID_INGRESS_PORT)
	{
		tmp |= r->ingress_port;
	}
	Write_Reg(P2REG_ACL_TABLE_DATA_EC, tmp);
	//ED
	tmp = ((ACL_SELECT_MODE_1011<<3) | (link<<1));
	Write_Reg(P2REG_ACL_TABLE_DATA_ED, tmp);

	Write_Reg(P2REG_ACL_TABLE_ACCESS, 0xC000|index);
	return 0;
}

int acl_write_table_1100(int index, int link, struct acl_rule *r)
{
	u16 tmp = 0;

	IP2Page(2);
	//E4 ~ EB
	Write_Reg(P2REG_ACL_TABLE_DATA_E4, r->sip6_addr16[7]);
	Write_Reg(P2REG_ACL_TABLE_DATA_E5, r->sip6_addr16[6]);
	Write_Reg(P2REG_ACL_TABLE_DATA_E6, r->sip6_addr16[5]);
	Write_Reg(P2REG_ACL_TABLE_DATA_E7, r->sip6_addr16[4]);
	Write_Reg(P2REG_ACL_TABLE_DATA_E8, r->sip6_addr16[3]);
	Write_Reg(P2REG_ACL_TABLE_DATA_E9, r->sip6_addr16[2]);
	Write_Reg(P2REG_ACL_TABLE_DATA_EA, r->sip6_addr16[1]);
	Write_Reg(P2REG_ACL_TABLE_DATA_EB, r->sip6_addr16[0]);
	//EC
	tmp |= ((u16)(r->sip_mask&0xF)<<8);
	if(r->rule_valid & ACL_RULE_VALID_INGRESS_PORT)
	{
		tmp |= r->ingress_port;
	}
	Write_Reg(P2REG_ACL_TABLE_DATA_EC, tmp);
	//ED
	tmp = ((ACL_SELECT_MODE_1100<<3) | (link<<1));
	Write_Reg(P2REG_ACL_TABLE_DATA_ED, tmp);

	Write_Reg(P2REG_ACL_TABLE_ACCESS, 0xC000|index);
	return 0;
}

int acl_write_table_1110(int index, int link, struct acl_rule *r)
{
	u16 tmp = 0;

	IP2Page(2);
	//E4 ~ EB
	Write_Reg(P2REG_ACL_TABLE_DATA_E4, r->dip6_addr16[7]);
	Write_Reg(P2REG_ACL_TABLE_DATA_E5, r->dip6_addr16[6]);
	Write_Reg(P2REG_ACL_TABLE_DATA_E6, r->dip6_addr16[5]);
	Write_Reg(P2REG_ACL_TABLE_DATA_E7, r->dip6_addr16[4]);
	Write_Reg(P2REG_ACL_TABLE_DATA_E8, r->dip6_addr16[3]);
	Write_Reg(P2REG_ACL_TABLE_DATA_E9, r->dip6_addr16[2]);
	Write_Reg(P2REG_ACL_TABLE_DATA_EA, r->dip6_addr16[1]);
	Write_Reg(P2REG_ACL_TABLE_DATA_EB, r->dip6_addr16[0]);
	//EC
	tmp |= ((u16)(r->dip_mask&0xF)<<8);
	if(r->rule_valid & ACL_RULE_VALID_INGRESS_PORT)
	{
		tmp |= r->ingress_port;
	}
	Write_Reg(P2REG_ACL_TABLE_DATA_EC, tmp);
	//ED
	tmp = ((ACL_SELECT_MODE_1110<<3) | (link<<1));
	Write_Reg(P2REG_ACL_TABLE_DATA_ED, tmp);

	Write_Reg(P2REG_ACL_TABLE_ACCESS, 0xC000|index);
	return 0;
}

int acl_write_table_1111(int index, int link, struct acl_rule *r)
{
	ip1829drv_dbg("<SW_Driver ACL> table mode 1111 is unused.\n");

	return 0;
}

int acl_write_table_drop(int index, struct acl_rule *r)
{
	u16 tmp = 0;

	IP2Page(2);
	//E4
	tmp = 0x1F;
	Write_Reg(P2REG_ACL_TABLE_DATA_E4, tmp);
	//E5
	tmp = 3;
	Write_Reg(P2REG_ACL_TABLE_DATA_E5, tmp);

	Write_Reg(P2REG_ACL_TABLE_ACCESS, 0xC100|index);
	return 0;
}

int acl_write_table_a1(int index, struct acl_rule *r)
{
	u16 tmp = 0;

	IP2Page(2);
	//E4
	if(r->act_valid & ACL_ACT_VALID_REDIR)
	{
		tmp |= (r->redir&0x1F);
	}
	if(r->act_valid & ACL_ACT_VALID_PRI)
	{
		tmp |= (((0x1<<3)|(r->pri&0x7))<<5);
	}
	if(r->act_valid & ACL_ACT_VALID_DSCP)
	{
		tmp |= (((0x1<<3)|(r->a_dscp&0x7))<<9);
	}
	if(r->act_valid & ACL_ACT_VALID_CPU)
	{
		tmp |= BIT(13);
	}
	if(r->act_valid & ACL_ACT_VALID_SNIFFER)
	{
		tmp |= BIT(14);
	}

	Write_Reg(P2REG_ACL_TABLE_DATA_E4, tmp);
	//E5
	tmp = 3;
	Write_Reg(P2REG_ACL_TABLE_DATA_E5, tmp);

	Write_Reg(P2REG_ACL_TABLE_ACCESS, 0xC100|index);
	return 0;
}

int acl_write_table_a2(int index, struct acl_rule *r)
{
	u16 tmp = 0;

	IP2Page(2);
	//E4
	if(r->act_valid & ACL_ACT_VALID_REDIR)
	{
		tmp |= (r->redir&0x1F);
	}
	if(r->act_valid & ACL_ACT_VALID_PRI)
	{
		tmp |= (((0x1<<3)|(r->pri&0x7))<<5);
	}
	if(r->act_valid & ACL_ACT_VALID_BW)
	{
		tmp |= ((r->bw&0xF)<<9);
	}
	if(r->act_valid & ACL_ACT_VALID_CPU)
	{
		tmp |= BIT(13);
	}
	if(r->act_valid & ACL_ACT_VALID_SFLOW)
	{
		tmp |= BIT(14);
	}
	if(r->act_valid & ACL_ACT_VALID_PTP)
	{
		tmp |= BIT(15);
	}

	Write_Reg(P2REG_ACL_TABLE_DATA_E4, tmp);
	//E5
	tmp = 2;
	Write_Reg(P2REG_ACL_TABLE_DATA_E5, tmp);

	Write_Reg(P2REG_ACL_TABLE_ACCESS, 0xC100|index);
	return 0;
}

int acl_write_table_a3(int index, struct acl_rule *r)
{
	u16 tmp = 0;

	IP2Page(2);
	//E4
	if(r->act_valid & ACL_ACT_VALID_REDIR)
	{
		tmp |= (r->redir&0x1F);
	}
	if(r->act_valid & ACL_ACT_VALID_PRI)
	{
		tmp |= (((0x1<<3)|(r->pri&0x7))<<5);
	}
	if(r->act_valid & ACL_ACT_VALID_CTAG)
	{
		tmp |= (((0x1<<6)|(r->ctag&0x1F))<<9);

		if(r->act_valid & ACL_ACT_VALID_CTAG_VLAN)
		{
			tmp |= BIT(14);
		}
	}

	Write_Reg(P2REG_ACL_TABLE_DATA_E4, tmp);
	//E5
	tmp = 1;
	Write_Reg(P2REG_ACL_TABLE_DATA_E5, tmp);

	Write_Reg(P2REG_ACL_TABLE_ACCESS, 0xC100|index);
	return 0;
}

int acl_write_table_a4(int index, struct acl_rule *r)
{
	u16 tmp = 0;

	IP2Page(2);
	//E4
	if(r->act_valid & ACL_ACT_VALID_CTAG)
	{
		tmp |= ((0x1<<6)|(r->ctag&0x1F));

		if(r->act_valid & ACL_ACT_VALID_CTAG_VLAN)
		{
			tmp |= BIT(5);
		}
	}
	if(r->act_valid & ACL_ACT_VALID_STAG)
	{
		tmp |= (((0x1<<6)|(r->stag&0x1F))<<7);

		if(r->act_valid & ACL_ACT_VALID_STAG_VLAN)
		{
			tmp |= BIT(12);
		}
	}
	if(r->act_valid & ACL_ACT_VALID_SNIFFER)
	{
		tmp |= BIT(14);
	}

	Write_Reg(P2REG_ACL_TABLE_DATA_E4, tmp);
	//E5
	tmp = 0;
	Write_Reg(P2REG_ACL_TABLE_DATA_E5, tmp);

	Write_Reg(P2REG_ACL_TABLE_ACCESS, 0xC100|index);
	return 0;
}

int acl_write_table_a5(int index, struct acl_rule *r)
{
	u16 tmp = 0;

	IP2Page(2);
	//E4
	if(r->act_valid & ACL_ACT_VALID_REDIR)
	{
		tmp |= (r->redir&0x1F);
	}
	if(r->act_valid & ACL_ACT_VALID_PRI)
	{
		tmp |= (((0x1<<3)|(r->pri&0x7))<<5);
	}
	if(r->act_valid & ACL_ACT_VALID_DSCP)
	{
		tmp |= (((0x1<<3)|(r->a_dscp&0x7))<<9);
	}
	if(r->act_valid & ACL_ACT_VALID_CPU)
	{
		tmp |= BIT(13);
	}
	if(r->act_valid & ACL_ACT_VALID_SNIFFER)
	{
		tmp |= BIT(14);
	}

	Write_Reg(P2REG_ACL_TABLE_DATA_E4, tmp);
	//E5
	tmp = 0;
	if(r->act_valid & ACL_ACT_VALID_CTAG)
	{
		tmp |= ((0x1<<6)|(r->ctag&0x1F));

		if(r->act_valid & ACL_ACT_VALID_CTAG_VLAN)
		{
			tmp |= BIT(5);
		}
	}
	if(r->act_valid & ACL_ACT_VALID_STAG)
	{
		tmp |= (((0x1<<6)|(r->stag&0x1F))<<7);

		if(r->act_valid & ACL_ACT_VALID_STAG_VLAN)
		{
			tmp |= BIT(12);
		}
	}
	if(r->act_valid & ACL_ACT_VALID_BW)
	{
		tmp |= ((r->bw&0x3)<<14);
	}

	Write_Reg(P2REG_ACL_TABLE_DATA_E5, tmp);
	//E6
	tmp = 0;
	if(r->act_valid & ACL_ACT_VALID_BW)
	{
		tmp |= ((r->bw&0xC)>>2);
	}
	if(r->act_valid & ACL_ACT_VALID_SFLOW)
	{
		tmp |= BIT(2);
	}
	if(r->act_valid & ACL_ACT_VALID_PTP)
	{
		tmp |= BIT(3);
	}

	Write_Reg(P2REG_ACL_TABLE_DATA_E6, tmp);
	//E7
	Write_Reg(P2REG_ACL_TABLE_DATA_E7, 0);
	//E8
	Write_Reg(P2REG_ACL_TABLE_DATA_E8, 0);
	//E9
	Write_Reg(P2REG_ACL_TABLE_DATA_E9, 0);
	//EA
	Write_Reg(P2REG_ACL_TABLE_DATA_EA, 0);
	//EB
	Write_Reg(P2REG_ACL_TABLE_DATA_EB, 0);
	//EC
	Write_Reg(P2REG_ACL_TABLE_DATA_EC, 0);
	//ED
	Write_Reg(P2REG_ACL_TABLE_DATA_ED, 0);

	tmp = ((ACL_SELECT_MODE_0001<<3) | (ACL_LINK_TYPE_11<<1));
	Write_Reg(P2REG_ACL_TABLE_DATA_ED, tmp);

	Write_Reg(P2REG_ACL_TABLE_ACCESS, 0xC000|index);
	return 0;
}

int setAclRule(void *cdata, int len)
{
	struct AclRuleSetting *ars = (struct AclRuleSetting *)cdata;
	struct acl_rule *r = &ars ->rule;
	unsigned long rule_v = r->rule_valid;
	int s_mode = 0, s_num = 0, flag = 0, i = 0, index = 0, link = 0;
	struct acl_man_rule *mr;

	ip1829drv_dbg("<DBG_DR_ACL> setAclRule(): ars ->index = %i\n", ars ->index);
	// check index
	if(ars ->index != 128)
	{
		list_for_each_entry(mr, &m_acl ->rule_list, rule_entry)
		{
			if(mr->start_index == ars ->index)
			{
				for(i = mr->start_index; i<(mr->start_index+mr->num_entries); i++)
				{
					acl_clean_table(i);
					m_acl ->used_entry_mask[i/16] &= ~BIT(i%16);
					m_acl ->num_used_entries--;
				}
				m_acl ->num_used_rules--;
				flag = 1;
				break;
			}
		}

		if(!flag)
		{
			ars ->index = -1;
			ars ->reserved = 0;
			return 0;
			//return -EINVAL;
		}
	}
	else
	{
		mr = NULL;
	}

	if(rule_v==0){
		ars->index=128;
		ars->reserved=0;
		m_acl->num_used_rules++;
		if(mr!=NULL){
			list_del(&mr->rule_entry);
			kfree(mr);
		}
		return 0;
	}

	// selection logic
	if(rule_v & ACL_RULE_VALID_SIP6)
	{
		// 1100
		s_mode |= ACL_SELECT_MODE_BIT_1100;
		s_num += 1;

		if(rule_v & ACL_RULE_VALID_DIP6)
		{
			// 1110
			s_mode |= ACL_SELECT_MODE_BIT_1110;
			s_num += 1;
		}

		if((rule_v & ACL_RULE_VALID_SP) || (rule_v & ACL_RULE_VALID_DP))
		{
			// 0011
			s_mode |= ACL_SELECT_MODE_BIT_0011;
			s_num += 1;
		}
		goto out_select_logic;
	}
	else if(rule_v & ACL_RULE_VALID_DIP6)
	{
		// 1110
		s_mode |= ACL_SELECT_MODE_BIT_1110;
		s_num += 1;

		if((rule_v & ACL_RULE_VALID_SP) || (rule_v & ACL_RULE_VALID_DP))
		{
			// 0011
			s_mode |= ACL_SELECT_MODE_BIT_0011;
			s_num += 1;
		}
		goto out_select_logic;
	}

	if((rule_v & ACL_RULE_VALID_TCP_FLAG) || (rule_v & ACL_RULE_VALID_SP) || (rule_v & ACL_RULE_VALID_DP))
	{
		// 0011
		s_mode |= ACL_SELECT_MODE_BIT_0011;
		s_num += 1;

		if((rule_v & ACL_RULE_VALID_SP_R) || (rule_v & ACL_RULE_VALID_DP_R))
		{
			// 0010
			s_mode |= ACL_SELECT_MODE_BIT_0010;
			s_num += 1;

			if((rule_v & ACL_RULE_VALID_ETH_TYPE) || ((rule_v & ACL_RULE_VALID_SMAC) && (rule_v & ACL_RULE_VALID_DMAC)))
			{
				// 0110
				s_mode |= ACL_SELECT_MODE_BIT_0110;
				s_num += 1;

				if((rule_v & ACL_RULE_VALID_DSCP) || (rule_v & ACL_RULE_VALID_IP_PROT))
				{
					// 1000 | 1001 | 1010 | 1011
					s_mode |= ACL_SELECT_MODE_BIT_1000;
					s_num += 1;
				}
				ip1829drv_dbg("<DBG_SWD_ACL>\n");
				goto out_select_logic;
			}
			else
			{
				if((rule_v & ACL_RULE_VALID_DSCP) || (rule_v & ACL_RULE_VALID_IP_PROT))
				{
					if(rule_v & ACL_RULE_VALID_SMAC)
					{
						// 1000 | 1001
						s_mode |= ACL_SELECT_MODE_BIT_1000;
						s_num += 1;
					}
					else if(rule_v & ACL_RULE_VALID_SMAC)
					{
						// 1010 | 1011
						s_mode |= ACL_SELECT_MODE_BIT_1010;
						s_num += 1;
					}
					else
					{
						// 1000 | 1001 | 1010 | 1011
						s_mode |= ACL_SELECT_MODE_BIT_1000;
						s_num += 1;
					}
					ip1829drv_dbg("<DBG_SWD_ACL>\n");
					goto out_select_logic;
				}
				else
				{
					if((rule_v & ACL_RULE_VALID_VLAN) || (rule_v & ACL_RULE_VALID_SMAC) || (rule_v & ACL_RULE_VALID_DMAC))
					{
						// 0110
						s_mode |= ACL_SELECT_MODE_BIT_0110;
						s_num += 1;
					}
					ip1829drv_dbg("<DBG_SWD_ACL>\n");
					goto out_select_logic;
				}
			}
		}
		else
		{
			if((rule_v & ACL_RULE_VALID_ETH_TYPE) || ((rule_v & ACL_RULE_VALID_SMAC) && (rule_v & ACL_RULE_VALID_DMAC)))
			{
				// 0110
				s_mode |= ACL_SELECT_MODE_BIT_0110;
				s_num += 1;

				if((rule_v & ACL_RULE_VALID_DSCP) || (rule_v & ACL_RULE_VALID_IP_PROT))
				{
					// 1000 | 1001 | 1010 | 1011
					s_mode |= ACL_SELECT_MODE_BIT_1000;
					s_num += 1;
				}
				ip1829drv_dbg("<DBG_SWD_ACL>\n");
				goto out_select_logic;
			}
			else
			{
				if((rule_v & ACL_RULE_VALID_DSCP) || (rule_v & ACL_RULE_VALID_IP_PROT))
				{
					if(rule_v & ACL_RULE_VALID_SMAC)
					{
						// 1000 | 1001
						s_mode |= ACL_SELECT_MODE_BIT_1000;
						s_num += 1;
					}
					else if(rule_v & ACL_RULE_VALID_SMAC)
					{
						// 1010 | 1011
						s_mode |= ACL_SELECT_MODE_BIT_1010;
						s_num += 1;
					}
					else
					{
						// 1000 | 1001 | 1010 | 1011
						s_mode |= ACL_SELECT_MODE_BIT_1000;
						s_num += 1;
					}
					ip1829drv_dbg("<DBG_SWD_ACL>\n");
					goto out_select_logic;
				}
				else
				{
					if((rule_v & ACL_RULE_VALID_VLAN) || (rule_v & ACL_RULE_VALID_SMAC) || (rule_v & ACL_RULE_VALID_DMAC))
					{
						// 0110
						s_mode |= ACL_SELECT_MODE_BIT_0110;
						s_num += 1;
					}
					ip1829drv_dbg("<DBG_SWD_ACL>\n");
					goto out_select_logic;
				}
			}
		}
	}
	else
	{
		if((rule_v & ACL_RULE_VALID_SP_R) || (rule_v & ACL_RULE_VALID_DP_R))
		{
			// 0010
			s_mode |= ACL_SELECT_MODE_BIT_0010;
			s_num += 1;

			if((rule_v & ACL_RULE_VALID_ETH_TYPE) || ((rule_v & ACL_RULE_VALID_SMAC) && (rule_v & ACL_RULE_VALID_DMAC)))
			{
				// 0110
				s_mode |= ACL_SELECT_MODE_BIT_0110;
				s_num += 1;

				if((rule_v & ACL_RULE_VALID_DSCP) || (rule_v & ACL_RULE_VALID_IP_PROT))
				{
					// 1000 | 1001 | 1010 | 1011
					s_mode |= ACL_SELECT_MODE_BIT_1000;
					s_num += 1;
				}
				ip1829drv_dbg("<DBG_SWD_ACL>\n");
				goto out_select_logic;
			}
			else
			{
				if((rule_v & ACL_RULE_VALID_DSCP) || (rule_v & ACL_RULE_VALID_IP_PROT))
				{
					if(rule_v & ACL_RULE_VALID_SMAC)
					{
						// 1000 | 1001
						s_mode |= ACL_SELECT_MODE_BIT_1000;
						s_num += 1;
					}
					else if(rule_v & ACL_RULE_VALID_SMAC)
					{
						// 1010 | 1011
						s_mode |= ACL_SELECT_MODE_BIT_1010;
						s_num += 1;
					}
					else
					{
						// 1000 | 1001 | 1010 | 1011
						s_mode |= ACL_SELECT_MODE_BIT_1000;
						s_num += 1;
					}
					ip1829drv_dbg("<DBG_SWD_ACL>\n");
					goto out_select_logic;
				}
				else
				{
					if((rule_v & ACL_RULE_VALID_VLAN) || (rule_v & ACL_RULE_VALID_SMAC) || (rule_v & ACL_RULE_VALID_DMAC))
					{
						// 0110
						s_mode |= ACL_SELECT_MODE_BIT_0110;
						s_num += 1;
					}
					ip1829drv_dbg("<DBG_SWD_ACL>\n");
					goto out_select_logic;
				}
			}
		}
		else
		{
			if((rule_v & ACL_RULE_VALID_ETH_TYPE) || ((rule_v & ACL_RULE_VALID_SMAC) && (rule_v & ACL_RULE_VALID_DMAC)))
			{
				// 0110
				s_mode |= ACL_SELECT_MODE_BIT_0110;
				s_num += 1;

				if((rule_v & ACL_RULE_VALID_SIP) && (rule_v & ACL_RULE_VALID_DIP))
				{
					// 0010 | 0011
					s_mode |= ACL_SELECT_MODE_BIT_0010;
					s_num += 1;

					if((rule_v & ACL_RULE_VALID_DSCP) || (rule_v & ACL_RULE_VALID_IP_PROT))
					{
						// 1000 | 1001 | 1010 | 1011
						s_mode |= ACL_SELECT_MODE_BIT_1000;
						s_num += 1;
					}
				}
				else if(rule_v & ACL_RULE_VALID_SIP)
				{
					// 1000 | 1010
					s_mode |= ACL_SELECT_MODE_BIT_1000;
					s_num += 1;
				}
				else if(rule_v & ACL_RULE_VALID_DIP)
				{
					// 1001 | 1011
					s_mode |= ACL_SELECT_MODE_BIT_1001;
					s_num += 1;
				}
				else if((rule_v & ACL_RULE_VALID_DSCP) || (rule_v & ACL_RULE_VALID_IP_PROT))
				{
					// 1000 | 1001 | 1010 | 1011
					s_mode |= ACL_SELECT_MODE_BIT_1000;
					s_num += 1;
				}
				ip1829drv_dbg("<DBG_SWD_ACL>\n");
				goto out_select_logic;
			}
			else
			{
				if((rule_v & ACL_RULE_VALID_SIP) && (rule_v & ACL_RULE_VALID_DIP))
				{
					// 0010 | 0011
					s_mode |= ACL_SELECT_MODE_BIT_0010;
					s_num += 1;

					if(rule_v & ACL_RULE_VALID_SMAC)
					{
						// 1000 | 1001
						s_mode |= ACL_SELECT_MODE_BIT_1000;
						s_num += 1;
					}
					else if(rule_v & ACL_RULE_VALID_DMAC)
					{
						// 1010 | 1011
						s_mode |= ACL_SELECT_MODE_BIT_1010;
						s_num += 1;
					}
					else if((rule_v & ACL_RULE_VALID_VLAN) || (rule_v & ACL_RULE_VALID_DSCP) || (rule_v & ACL_RULE_VALID_IP_PROT))
					{
						// 1000 | 1001 | 1010 | 1011
						s_mode |= ACL_SELECT_MODE_BIT_1000;
						s_num += 1;
					}
					ip1829drv_dbg("<DBG_SWD_ACL>\n");
					goto out_select_logic;
				}
				else
				{
					if(rule_v & ACL_RULE_VALID_SMAC)
					{
						if(rule_v & ACL_RULE_VALID_SIP)
						{
							// 1000
							s_mode |= ACL_SELECT_MODE_BIT_1000;
							s_num += 1;
						}
						else if(rule_v & ACL_RULE_VALID_DIP)
						{
							// 1001
							s_mode |= ACL_SELECT_MODE_BIT_1001;
							s_num += 1;
						}
						else
						{
							// 1000 | 1001
							s_mode |= ACL_SELECT_MODE_BIT_1000;
							s_num += 1;
						}
					}
					else if(rule_v & ACL_RULE_VALID_DMAC)
					{
						if(rule_v & ACL_RULE_VALID_SIP)
						{
							// 1010
							s_mode |= ACL_SELECT_MODE_BIT_1010;
							s_num += 1;
						}
						else if(rule_v & ACL_RULE_VALID_DIP)
						{
							// 1011
							s_mode |= ACL_SELECT_MODE_BIT_1011;
							s_num += 1;
						}
						else
						{
							// 1010 | 1011
							s_mode |= ACL_SELECT_MODE_BIT_1010;
							s_num += 1;
						}
					}
					else if((rule_v & ACL_RULE_VALID_VLAN) || (rule_v & ACL_RULE_VALID_DSCP) || (rule_v & ACL_RULE_VALID_IP_PROT))
					{
						if(rule_v & ACL_RULE_VALID_SIP)
						{
							// 1000 | 1010
							s_mode |= ACL_SELECT_MODE_BIT_1000;
							s_num += 1;
						}
						else if(rule_v & ACL_RULE_VALID_DIP)
						{
							// 1001 | 1011
							s_mode |= ACL_SELECT_MODE_BIT_1001;
							s_num += 1;
						}
						else
						{
							// 1000 | 1001 | 1010 | 1011
							s_mode |= ACL_SELECT_MODE_BIT_1000;
							s_num += 1;
						}
					}
					else
					{
						// 0010 | 0011
						s_mode |= ACL_SELECT_MODE_BIT_0010;
						s_num += 1;
					}
					ip1829drv_dbg("<DBG_SWD_ACL>\n");
					goto out_select_logic;
				}
			}
		}
	}

out_select_logic:
	ip1829drv_dbg("<DBG_SWD_ACL> s_mode = %02x, s_num = %i\n", s_mode, s_num);

	if(r->act_type == ACL_ACT_TYPE_5)
	{
		s_num++;
	}

	index = acl_find_index(s_num);
	if(index < 0)
	{
		if(mr != NULL)
		{
			list_del(&mr->rule_entry);
			kfree(mr);
		}
		ars ->index = -2;
		ars ->reserved = s_num;
		return 0;
		//return -ENOMEM;
	}
	ip1829drv_dbg("<DBG_SWD_ACL> index = %i\n", index);

	if(mr == NULL)
	{
		mr = kmalloc(sizeof(struct acl_man_rule), GFP_KERNEL);
		if(mr == NULL)
		{
			ip1829drv_dbg("Error: setAclRule() acl_man_rule malloc failed.\n");
			return -ENOMEM;
		}
		list_add_tail(&mr->rule_entry, &m_acl ->rule_list);
	}


	if(r->act_type == ACL_ACT_TYPE_5)
	{
		mr->start_index = index;;
		mr->num_entries = s_num;

		link = ACL_LINK_TYPE_01;
		// write rule table
		for(i=2; i<16; i++)
		{
			if(s_mode & BIT(i))
			{
				acl_write_table_rule[i-1](index, link, r);
				index++;
				s_num--;

				if(s_num == 1)
				{
					break;
				}
				else
				{
					link = ACL_LINK_TYPE_10;
				}
			}
		}
		// write action table
		acl_write_table_act[r->act_type](index, r);
	}
	else
	{
		mr->start_index = index;;
		mr->num_entries = s_num;
		if(s_num == 1)
		{
			link = ACL_LINK_TYPE_00;
			// write rule table
			for(i=2; i<16; i++)
			{
				if(s_mode & BIT(i))
				{
					acl_write_table_rule[i-1](index, link, r);
					break;
				}
			}
			// write action table
			acl_write_table_act[r->act_type](index, r);
		}
		else
		{
			link = ACL_LINK_TYPE_01;
			// write rule table
			for(i=2; i<16; i++)
			{
				if(s_mode & BIT(i))
				{
					acl_write_table_rule[i-1](index, link, r);

					s_num--;
					if(s_num == 0)
					{
						break;
					}
					else if(s_num == 1)
					{
						link = ACL_LINK_TYPE_11;
					}
					else
					{
						link = ACL_LINK_TYPE_10;
					}
					index++;
				}
			}
			// write action table
			acl_write_table_act[r->act_type](index, r);
		}
	}

	for(i=0; i<mr->num_entries; i++)
	{
		m_acl ->used_entry_mask[(i+mr->start_index)/16] |= BIT((i+mr->start_index)%16);
		m_acl ->num_used_entries++;
	}
	m_acl ->num_used_rules++;
	ars ->index = mr->start_index;
	ars ->reserved = mr->num_entries;

	return 0;
}

int getAclRule(void *cdata, int len)
{
	return 0;
}

int aclCleanTable(void *cdata, int len)
{
	int i = 0;
	struct GeneralSetting *gs = (struct GeneralSetting *)cdata;
	struct acl_man_rule *mr, *nmr;

	ip1829drv_dbg("<DBG_DR_ACL> aclCleanTable()\n");

	if(gs ->gdata == 128)
	{
		ip1829drv_dbg("<DBG_DR_ACL> aclCleanTable() 111111\n");
		if(list_empty(&m_acl ->rule_list))
		{
			ip1829drv_dbg("<DBG_DR_ACL> aclCleanTable() 222222\n");
			return 0;
		}

		list_for_each_entry_safe(mr, nmr, &m_acl ->rule_list, rule_entry)
		{
			for(i = mr->start_index; i<(mr->start_index+mr->num_entries); i++)
			{
				acl_clean_table(i);
				m_acl ->used_entry_mask[i/16] &= ~BIT(i%16);
				m_acl ->num_used_entries--;
			}
			m_acl ->num_used_rules--;
			list_del(&mr->rule_entry);
			kfree(mr);
		}

		INIT_LIST_HEAD(&m_acl ->rule_list);

		if((m_acl ->num_used_rules != 0) || (m_acl ->num_used_entries != 0))
			ip1829drv_dbg("<DBG_SWD_ACL> aclCleanTable something error!\n");
	}
	else
	{
		list_for_each_entry(mr, &m_acl ->rule_list, rule_entry)
		{
			if(mr->start_index == gs ->gdata)
			{
				for(i = mr->start_index; i<(mr->start_index+mr->num_entries); i++)
				{
					acl_clean_table(i);
					m_acl ->used_entry_mask[i/16] &= ~BIT(i%16);
					m_acl ->num_used_entries--;
				}
				m_acl ->num_used_rules--;
				list_del(&mr->rule_entry);
				kfree(mr);
				break;
			}
		}
	}

	return 0;
}

int setAclFunctionEn(void *cdata, int len)
{
	struct GeneralSetting *gs = (struct GeneralSetting *)cdata;

	_WriteRegBits(1, P1REG_MISCCFG, 0, 1, gs ->gdata);
	return 0;
}

int getAclFunctionEn(void *cdata, int len)
{
	struct GeneralSetting *gs = (struct GeneralSetting *)cdata;

	gs ->gdata =_ReadRegBits(1, P1REG_MISCCFG, 0, 1);

	return 0;
}

int getAclUsedRules(void *cdata, int len)
{
	struct GeneralSetting *gs = (struct GeneralSetting *)cdata;

	gs ->gdata = m_acl ->num_used_rules;

	return 0;
}

int getAclUsedEntries(void *cdata, int len)
{
	struct GeneralSetting *gs = (struct GeneralSetting *)cdata;

	gs ->gdata = m_acl ->num_used_entries;

	return 0;
}

int setAclBW(void *cdata, int len)
{
	struct AclGeneralSetting *ags = (struct AclGeneralSetting *)cdata;
	u16 index = ags ->index;
	u16 rate = 0;
	u32 in_rate = ags ->data;

	if(ags ->data > 163)
	{
		rate |= BIT(7);
		rate |= (in_rate*100/2000);
		if(in_rate*100%2000) rate++;
	}
	else
	{
		rate = (in_rate*100/128);
		if(in_rate*100%128) rate++;
	}

	_WriteRegBits(2, P2REG_ACL_BW_01+(index/2), 8*(index%2), 8, rate);

	return 0;
}

int getAclBW(void *cdata, int len)
{
	struct AclGeneralSetting *ags = (struct AclGeneralSetting *)cdata;
	u16 index = ags ->index;
	u16 tmp = 0;
	u32 c_rate;

	tmp = _ReadRegBits(2, P2REG_ACL_BW_01+(index/2), (8*(index%2)), 8);

	if(tmp&BIT(7))
	{
		c_rate = (tmp&(0x7F))*2000;
	}
	else
	{
		c_rate = (tmp&(0x7F))*128;
	}

	ags ->data = c_rate/100;

	return 0;
}

int setAclDscp(void *cdata, int len)
{
	struct AclGeneralSetting *ags = (struct AclGeneralSetting *)cdata;
	u16 index = ags ->index;

	_WriteRegBits(7, P7REG_DSCP_REMARKING_01+(index/2), 8*(index%2), 8, ags ->data);

	return 0;
}

int getAclDscp(void *cdata, int len)
{
	struct AclGeneralSetting *ags = (struct AclGeneralSetting *)cdata;
	u16 index = ags ->index;

	ags ->data = _ReadRegBits(7, P7REG_DSCP_REMARKING_01+(index/2), (8*(index%2)), 8);

	return 0;
}

int setAclVlanGroup(void *cdata, int len)
{
	struct AclVlanGroupSetting *avg = (struct AclVlanGroupSetting *)cdata;
	u16 index = avg ->index;

	IP2Page(2);
	Write_Reg(P2REG_ACL_VLAN_01_0+(index*2), avg ->portmap&0xFFFF);
	Write_Reg(P2REG_ACL_VLAN_01_0+(index*2)+1, (avg ->portmap>>16)&0xFFFF);

	return 0;
}

int getAclVlanGroup(void *cdata, int len)
{
	struct AclVlanGroupSetting *avg = (struct AclVlanGroupSetting *)cdata;
	u16 index = avg ->index;

	IP2Page(2);
	avg ->portmap = Read_Reg(P2REG_ACL_VLAN_01_0+(index*2)) | (Read_Reg(P2REG_ACL_VLAN_01_0+(index*2)+1)<<16);

	return 0;
}

int setAclVidRemark(void *cdata, int len)
{
	struct AclGeneralSetting *ags = (struct AclGeneralSetting *)cdata;
	u16 index = ags ->index;

	if(index%2)
	{
		_WriteRegBits(2, P2REG_ACL_VID_REMARK_00+index-1, 15, 1, (ags ->data&0x1));
		_WriteRegBits(2, P2REG_ACL_VID_REMARK_00+index, 0, 14, (ags ->data>>1));
	}
	else
	{
		_WriteRegBits(2, P2REG_ACL_VID_REMARK_00+index, 0, 15, ags ->data);
	}

	return 0;
}

int getAclVidRemark(void *cdata, int len)
{
	struct AclGeneralSetting *ags = (struct AclGeneralSetting *)cdata;
	u16 index = ags ->index;

	if(index%2)
	{
		ags ->data = ((_ReadRegBits(2,P2REG_ACL_VID_REMARK_00+index,0,14)) <<1)| ((_ReadRegBits(2,P2REG_ACL_VID_REMARK_00+index-1,15,1)&0x1));	}
	else
	{
		ags ->data = _ReadRegBits(2, P2REG_ACL_VID_REMARK_00+index, 0, 15);
	}

	return 0;
}

//------------ ACL functions:common end  -----------------------
//------------ Bandwidth functions:common   --------------------
int setBandwidthIngressRate(void *cdata, int len)
{
	int port;	//1-28
	unsigned long rate;
	FUNC_MSG_IN;
	if (sizeof(struct ByPortSetting32) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}
	port = ((struct ByPortSetting32 *)cdata)->port;
	rate = ((struct ByPortSetting32 *)cdata)->pdata;
	if (port < 1 || port > SWITCH_MAX_IPORT_CNT)
	{
		ip1829drv_dbg("Error: port=%d\n", port);
		return -EINVAL;
	}
	if (rate > MAX_GIGA_SPEED)
	{
		ip1829drv_dbg("Error: pdata=0x%08X\n", (u16)rate);
		return -EINVAL;
	}
	IP2Page(0);
	if (rate == 0 || rate == MAX_GIGA_SPEED || (port <= MAX_PHY_TP_NUM && rate >= MAX_TP_SPEED))
	{	Write_Reg(P0REG_INGRESS_RATE_CTRL0+port-1,0);	}
	else if(rate > 0 && rate < MAX_GIGA_SPEED)
	{
		if(port <= MAX_PHY_TP_NUM && rate >= (MAX_TP_SPEED/RATE_SCALE_UNIT)*RATE_SCALE_UNIT)
		{	Write_Reg(P0REG_INGRESS_RATE_CTRL0+port-1,0);	}
		else if(rate >= (MAX_GIGA_SPEED/RATE_SCALE_UNIT)*RATE_SCALE_UNIT)
		{	Write_Reg(P0REG_INGRESS_RATE_CTRL0+port-1,0);	}
		else
		{
			rate=rate/RATE_SCALE_UNIT;
			if(rate==0)
				rate=1;
			Write_Reg(P0REG_INGRESS_RATE_CTRL0+port-1,rate);
		}
	}
	ip1829drv_dbg("cdata->port=%d\n", ((struct ByPortSetting32 *)cdata)->port);
	ip1829drv_dbg("cdata->pdata=0x%08X\n", (u16)((struct ByPortSetting32 *)cdata)->pdata);
	FUNC_MSG_OUT;
	return 0;
}

int getBandwidthIngressRate(void *cdata, int len)
{
	int port;
	FUNC_MSG_IN;
	if (sizeof(struct ByPortSetting32) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}
	port = ((struct ByPortSetting32 *)cdata)->port;
	if (port < 1 || port > SWITCH_MAX_IPORT_CNT)
	{
		ip1829drv_dbg("Error: port=%d\n", port);
		return -EINVAL;
	}
	((struct ByPortSetting32 *)cdata)->pdata =(unsigned long)(_ReadRegBits(0, P0REG_INGRESS_RATE_CTRL0+port-1, 0, 14)*64000);
	ip1829drv_dbg("cdata->port=%d\n", ((struct ByPortSetting32 *)cdata)->port);
	ip1829drv_dbg("cdata->pdata=0x%08X\n", (u16)((struct ByPortSetting32 *)cdata)->pdata);
	FUNC_MSG_OUT;
	return 0;
}

int setBandwidthEgressRate(void *cdata, int len)
{
	int port;
	unsigned long rate;
	FUNC_MSG_IN;
	if (sizeof(struct ByPortSetting32) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}
	port = ((struct ByPortSetting32 *)cdata)->port;
	rate = ((struct ByPortSetting32 *)cdata)->pdata;
	
	if (port < 1 || port > SWITCH_MAX_IPORT_CNT)
	{
		ip1829drv_dbg("Error: port=%d\n", port);
		return -EINVAL;
	}
	if (rate > MAX_GIGA_SPEED)
	{
		ip1829drv_dbg("Error: pdata=0x%08X\n", (u16)rate);
		return -EINVAL;
	}
	IP2Page(8);
	if (rate == 0 || rate == MAX_GIGA_SPEED || (port <= MAX_PHY_TP_NUM && rate >= MAX_TP_SPEED))
	{	Write_Reg(P8REG_EGRESS_RATE_CTRL0+port-1,0);	}
	else if(rate > 0 && rate < MAX_GIGA_SPEED)
	{
		if(port <= MAX_PHY_TP_NUM && rate >= (MAX_TP_SPEED/RATE_SCALE_UNIT)*RATE_SCALE_UNIT)
		{	Write_Reg(P8REG_EGRESS_RATE_CTRL0+port-1,0);	}
		else if(rate >= (MAX_GIGA_SPEED/RATE_SCALE_UNIT)*RATE_SCALE_UNIT)
		{	Write_Reg(P8REG_EGRESS_RATE_CTRL0+port-1,0);	}
		else
		{
			rate=rate/RATE_SCALE_UNIT;
			if(rate==0)
				rate=1;
			Write_Reg(P8REG_EGRESS_RATE_CTRL0+port-1,rate/RATE_SCALE_UNIT+1);
		}
	}
	ip1829drv_dbg("cdata->port=%d\n", ((struct ByPortSetting32 *)cdata)->port);
	ip1829drv_dbg("cdata->pdata=0x%08X\n", (u16)((struct ByPortSetting32 *)cdata)->pdata);
	FUNC_MSG_OUT;
	return 0;
}

int getBandwidthEgressRate(void *cdata, int len)
{
	int port;
	FUNC_MSG_IN;
	if (sizeof(struct ByPortSetting32) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}
	port = ((struct ByPortSetting32 *)cdata)->port;
	if (port < 1 || port > SWITCH_MAX_IPORT_CNT)
	{
		ip1829drv_dbg("Error: port=%d\n", port);
		return -EINVAL;
	}
	((struct ByPortSetting32 *)cdata)->pdata =(unsigned long)(_ReadRegBits(8, P8REG_EGRESS_RATE_CTRL0+port-1, 0, 14)*64000);
	ip1829drv_dbg("cdata->port=%d\n", ((struct ByPortSetting32 *)cdata)->port);
	ip1829drv_dbg("cdata->pdata=0x%08X\n", (u16)((struct ByPortSetting32 *)cdata)->pdata);
	FUNC_MSG_OUT;
	return 0;
}

int setBandwidthEgressPeriod(void *cdata, int len)
{
	int period;
	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}
	period = ((struct GeneralSetting *)cdata)->gdata;
	if (period < 0 || period > 7)
	{
		ip1829drv_dbg("Error: gdata=%d\n", period);
		return -EINVAL;
	}
	_WriteRegBits(8, P8REG_OUT_QUEUE_PARAM, 4, 3, period);
	ip1829drv_dbg("cdata->gdata=0x%08X\n", ((struct GeneralSetting *)cdata)->gdata);
	FUNC_MSG_OUT;
	return 0;
}

int getBandwidthEgressPeriod(void *cdata, int len)
{
	int period;
	FUNC_MSG_IN;
	if (sizeof(struct GeneralSetting) != len)
	{
		ip1829drv_dbg("Error: lengtn=%d\n", len);
		return -EINVAL;
	}
	period = ((struct GeneralSetting *)cdata)->gdata;
	if (period < 0 || period > 7)
	{
		ip1829drv_dbg("Error: gdata=%d\n", period);
		return -EINVAL;
	}
	((struct GeneralSetting *)cdata)->gdata = _ReadRegBits(8,P8REG_OUT_QUEUE_PARAM,4,3);
	ip1829drv_dbg("cdata->gdata=0x%08X\n", ((struct GeneralSetting *)cdata)->gdata);
	FUNC_MSG_OUT;
	return 0;
}
//------------ Bandwidth functions:common end  -----------------
int setMdioDivisor(void *cdata, int len)
{
	void __iomem	*base;
	unsigned long val;
	struct ip218_smictrl1 *smic1;
	struct GeneralSetting *gs = (struct GeneralSetting *)cdata;

	FUNC_MSG_IN;

	base = (void __iomem *)(IP218_MAC_BASE + IP218_MAC_SMICTRL1);
	val = 0;
	smic1 = (struct ip218_smictrl1 *)&val;
	smic1 ->mdc_clk_divisor = gs ->gdata;
	writel(val, base);

	val = readl(base);
	smic1 = (struct ip218_smictrl1 *)&val;
	ip1829drv_dbg("ip218 mdio divisor=%u\n", (unsigned short)smic1 ->mdc_clk_divisor);

	FUNC_MSG_OUT;
	return 0;
}

