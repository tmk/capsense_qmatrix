#include "util.h"
#include "descriptor.h"


/*******************************************************************************
 * HID Report Descriptors
 ******************************************************************************/
const USB_Descriptor_HIDReport_Datatype_t PROGMEM ConsoleReport[] =
{
    HID_RI_USAGE_PAGE(16, 0xFF31), /* Vendor Page(PJRC Teensy compatible) */
    HID_RI_USAGE(8, 0x74), /* Vendor Usage(PJRC Teensy compatible) */
    HID_RI_COLLECTION(8, 0x01), /* Application */
        HID_RI_USAGE(8, 0x75), /* Vendor Usage 0x75 */
        HID_RI_LOGICAL_MINIMUM(8, 0x00),
        HID_RI_LOGICAL_MAXIMUM(8, 0xFF),
        HID_RI_REPORT_COUNT(8, CONSOLE_EPSIZE),
        HID_RI_REPORT_SIZE(8, 0x08),
        HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
        HID_RI_USAGE(8, 0x76), /* Vendor Usage 0x76 */
        HID_RI_LOGICAL_MINIMUM(8, 0x00),
        HID_RI_LOGICAL_MAXIMUM(8, 0xFF),
        HID_RI_REPORT_COUNT(8, CONSOLE_EPSIZE),
        HID_RI_REPORT_SIZE(8, 0x08),
        HID_RI_OUTPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE | HID_IOF_NON_VOLATILE),
    HID_RI_END_COLLECTION(0),
};


/*******************************************************************************
 * Device Descriptors
 ******************************************************************************/
const USB_Descriptor_Device_t PROGMEM DeviceDescriptor =
{
    .Header                 = {.Size = sizeof(USB_Descriptor_Device_t), .Type = DTYPE_Device},

    .USBSpecification       = VERSION_BCD(1,1,0),
    .Class                  = USB_CSCP_NoDeviceClass,
    .SubClass               = USB_CSCP_NoDeviceSubclass,
    .Protocol               = USB_CSCP_NoDeviceProtocol,

    .Endpoint0Size          = FIXED_CONTROL_ENDPOINT_SIZE,

    /* specified in config.h */
    .VendorID               = VENDOR_ID,
    .ProductID              = PRODUCT_ID,
    .ReleaseNumber          = DEVICE_VER,

    .ManufacturerStrIndex   = 0x01,
    .ProductStrIndex        = 0x02,
    .SerialNumStrIndex      = NO_DESCRIPTOR,

    .NumberOfConfigurations = FIXED_NUM_CONFIGURATIONS
};

/*******************************************************************************
 * Configuration Descriptors
 ******************************************************************************/
const USB_Descriptor_Configuration_t PROGMEM ConfigurationDescriptor =
{
    .Config =
        {
            .Header                 = {.Size = sizeof(USB_Descriptor_Configuration_Header_t), .Type = DTYPE_Configuration},

            .TotalConfigurationSize = sizeof(USB_Descriptor_Configuration_t),
            .TotalInterfaces        = TOTAL_INTERFACES,

            .ConfigurationNumber    = 1,
            .ConfigurationStrIndex  = NO_DESCRIPTOR,

            .ConfigAttributes       = (USB_CONFIG_ATTR_RESERVED | USB_CONFIG_ATTR_REMOTEWAKEUP),

            .MaxPowerConsumption    = USB_CONFIG_POWER_MA(100)
        },

    /*
     * Console
     */
    .Console_Interface =
        {
            .Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},

            .InterfaceNumber        = CONSOLE_INTERFACE,
            .AlternateSetting       = 0x00,

            .TotalEndpoints         = 2,

            .Class                  = HID_CSCP_HIDClass,
            .SubClass               = HID_CSCP_NonBootSubclass,
            .Protocol               = HID_CSCP_NonBootProtocol,

            .InterfaceStrIndex      = NO_DESCRIPTOR
        },

    .Console_HID =
        {
            .Header                 = {.Size = sizeof(USB_HID_Descriptor_HID_t), .Type = HID_DTYPE_HID},

            .HIDSpec                = VERSION_BCD(1,1,1),
            .CountryCode            = 0x00,
            .TotalReportDescriptors = 1,
            .HIDReportType          = HID_DTYPE_Report,
            .HIDReportLength        = sizeof(ConsoleReport)
        },

    .Console_INEndpoint =
        {
            .Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

            .EndpointAddress        = (ENDPOINT_DIR_IN | CONSOLE_IN_EPNUM),
            .Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
            .EndpointSize           = CONSOLE_EPSIZE,
            .PollingIntervalMS      = 0x01
        },

    .Console_OUTEndpoint =
        {
            .Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

            .EndpointAddress        = (ENDPOINT_DIR_OUT | CONSOLE_OUT_EPNUM),
            .Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
            .EndpointSize           = CONSOLE_EPSIZE,
            .PollingIntervalMS      = 0x01
        },
};


/*******************************************************************************
 * String Descriptors
 ******************************************************************************/
const USB_Descriptor_String_t PROGMEM LanguageString =
{
    .Header                 = {.Size = USB_STRING_LEN(1), .Type = DTYPE_String},

    .UnicodeString          = {LANGUAGE_ID_ENG}
};

const USB_Descriptor_String_t PROGMEM ManufacturerString =
{
    /* subtract 1 for null terminator */
    .Header                 = {.Size = USB_STRING_LEN(sizeof(STR(MANUFACTURER))-1), .Type = DTYPE_String},

    .UnicodeString          = LSTR(MANUFACTURER)
};

const USB_Descriptor_String_t PROGMEM ProductString =
{
    /* subtract 1 for null terminator */
    .Header                 = {.Size = USB_STRING_LEN(sizeof(STR(PRODUCT))-1), .Type = DTYPE_String},

    .UnicodeString          = LSTR(PRODUCT)
};


/** This function is called by the library when in device mode, and must be overridden (see library "USB Descriptors"
 *  documentation) by the application code so that the address and size of a requested descriptor can be given
 *  to the USB library. When the device receives a Get Descriptor request on the control endpoint, this function
 *  is called so that the descriptor details can be passed back and the appropriate descriptor sent back to the
 *  USB host.
 */
uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue,
                                    const uint8_t wIndex,
                                    const void** const DescriptorAddress)
{
    const uint8_t  DescriptorType   = (wValue >> 8);
    const uint8_t  DescriptorIndex  = (wValue & 0xFF);

    const void* Address = NULL;
    uint16_t    Size    = NO_DESCRIPTOR;

    switch (DescriptorType)
    {
        case DTYPE_Device:
            Address = &DeviceDescriptor;
            Size    = sizeof(USB_Descriptor_Device_t);
            break;
        case DTYPE_Configuration:
            Address = &ConfigurationDescriptor;
            Size    = sizeof(USB_Descriptor_Configuration_t);
            break;
        case DTYPE_String:
            switch (DescriptorIndex )
            {
                case 0x00:
                    Address = &LanguageString;
                    Size    = pgm_read_byte(&LanguageString.Header.Size);
                    break;
                case 0x01:
                    Address = &ManufacturerString;
                    Size    = pgm_read_byte(&ManufacturerString.Header.Size);
                    break;
                case 0x02:
                    Address = &ProductString;
                    Size    = pgm_read_byte(&ProductString.Header.Size);
                    break;
            }
            break;
        case HID_DTYPE_HID:
            switch (wIndex) {
            case CONSOLE_INTERFACE:
                Address = &ConfigurationDescriptor.Console_HID;
                Size    = sizeof(USB_HID_Descriptor_HID_t);
                break;
            }
            break;
        case HID_DTYPE_Report:
            switch (wIndex) {
            case CONSOLE_INTERFACE:
                Address = &ConsoleReport;
                Size    = sizeof(ConsoleReport);
                break;
            }
            break;
    }

    *DescriptorAddress = Address;
    return Size;
}
