#include "xv_frmbufwr.h"

XV_frmbufwr_Config XV_frmbufwr_ConfigTable[] __attribute__ ((section (".drvcfg_sec"))) = {

	{
		"xlnx,v-frmbuf-wr-2.5", /* compatible */
		0xa0010000, /* reg */
		0x2, /* xlnx,samples-per-clock */
		0xf00, /* xlnx,max-cols */
		0x870, /* xlnx,max-rows */
		0xa, /* xlnx,max-data-width */
		0x80, /* xlnx,aximm-data-width */
		0x20, /* xlnx,aximm-addr-width */
		0x0, /* xlnx,has-rgbx8 */
		0x0, /* xlnx,has-yuvx8 */
		0x0, /* xlnx,has-yuyv8 */
		0x0, /* xlnx,has-rgbx10 */
		0x0, /* xlnx,has-yuvx10 */
		0x1, /* xlnx,has-y-uv8 */
		0x1, /* xlnx,has-y-uv8-420 */
		0x0, /* xlnx,has-rgb8 */
		0x0, /* xlnx,has-yuv8 */
		0x1, /* xlnx,has-y-uv10 */
		0x1, /* xlnx,has-y-uv10-420 */
		0x1, /* xlnx,has-y8 */
		0x1, /* xlnx,has-y10 */
		0x0, /* xlnx,has-bgrx8 */
		0x0, /* xlnx,has-uyvy8 */
		0x0, /* xlnx,has-bgr8 */
		0x0, /* xlnx,has-rgbx12 */
		0x0, /* xlnx,has-rgb16 */
		0x0, /* xlnx,has-yuvx12 */
		0x0, /* xlnx,has-y-uv12 */
		0x0, /* xlnx,has-y-uv12-420 */
		0x0, /* xlnx,has-y12 */
		0x0, /* xlnx,has-yuv16 */
		0x0, /* xlnx,has-y-uv16 */
		0x0, /* xlnx,has-y-uv16-420 */
		0x0, /* xlnx,has-y16 */
		0x0, /* xlnx,has-y-u-v8 */
		0x0, /* xlnx,has-y-u-v10 */
		0x0, /* xlnx,has-y-u-v8-420 */
		0x0, /* xlnx,has-interlaced */
		0x405b, /* interrupts */
		0xf9010000 /* interrupt-parent */
	},
	 {
		 NULL
	}
};