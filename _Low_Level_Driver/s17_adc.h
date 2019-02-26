#ifndef __DEF_ADC10
#define	__DEF_ADC10

typedef enum
{
    AN0     = 0x0001,
    AN1     = 0x0002,
    AN2     = 0x0004,
    AN3     = 0x0008,
    AN4     = 0x0010,
    AN5     = 0x0020,
    AN6     = 0x0040,
    AN7     = 0x0080,
    AN8     = 0x0100,
    AN9     = 0x0200,
    AN10    = 0x0400,
    AN11    = 0x0800,
    AN12    = 0x1000,
    AN13    = 0x2000,
    AN14    = 0x4000,
    AN15    = 0x8000
} ADC10_ANALOG_PIN;

typedef enum
{
	ADC10_VDD_VSS       = 0,
    ADC10_VREFP_VSS     = (1 << _AD1CON2_VCFG_POSITION),
    ADC10_VDD_VREFN     = (2 << _AD1CON2_VCFG_POSITION),
    ADC10_VREFP_VREFN   = (3 << _AD1CON2_VCFG_POSITION)
} ADC10_VOLTAGE_REF;

#define ADC_MODULE_ON_MASK          _AD1CON1_ON_MASK
#define ADC_MODULE_ON               (1 << _AD1CON1_ADON_POSITION)
#define ADC_FORMAT_INTG16           (0x00 << _AD1CON1_FORM_POSITION)
#define ADC_CLK_AUTO                (7 << _AD1CON1_SSRC_POSITION)
#define ADC_AUTO_SAMPLING_ON        (1 << _AD1CON1_ASAM_POSITION) 
#define ADC_SCAN_ON                 (1 << _AD1CON2_CSCNA_POSITION)
#define ADC_CONV_CLK_INTERNAL_RC    (1 << _AD1CON3_ADRC_POSITION)
#define ADC_SAMPLE_TIME_15          (0x0F << _AD1CON3_SAMC_POSITION)

void adc10_init(ADC10_ANALOG_PIN channels, ADC10_VOLTAGE_REF vref);
uint16_t adc10_read(ADC10_ANALOG_PIN channel);

#endif
