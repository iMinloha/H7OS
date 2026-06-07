#include "bsp_adc.h"
#include "Core/DrT.h"
#include "memctl.h"
#include <stdio.h>

#include "usbd_cdc_if.h"

static int adc_read(void *dev, uint8_t *buf, uint32_t len) {
    ADC_HandleTypeDef *h = (ADC_HandleTypeDef*)dev;
    HAL_ADCEx_Calibration_Start(h, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED);
    HAL_ADC_Start(h);
    HAL_StatusTypeDef s = HAL_ADC_PollForConversion(h, 10);
    if (s == HAL_OK) {
        uint32_t val = HAL_ADC_GetValue(h);
        int n = snprintf((char*)buf, len, "%lu", val);
        HAL_ADC_Stop(h);
        return n;
    }
    HAL_ADC_Stop(h);
    USB_printf("[ADC] poll timeout\n");
    return -1;
}

static int adc_write(void *dev, const uint8_t *buf, uint32_t len) {
    (void)dev; (void)buf; (void)len; return -1;
}

static bsp_file_ops_t adc_fops = { NULL, NULL, adc_read, adc_write };

void adc_register(ADC_HandleTypeDef *hadc, const char *name) {
    addDevice("dev/adc", hadc, (char*)name, "ADC input", DEVICE_VOTAGE, DEVICE_ON, NULL);
    char _path[64]; sprintf(_path, "/dev/adc/%s", name);
    DrTNode_t d = loadDevice(_path);
    if (d) d->fops = (void*)&adc_fops;
}

void adc_device_init(void) {
    ADC_REGISTER(hadc1, "ADC1");
}
