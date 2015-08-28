#ifndef _DESCRIPTORS_H_
#define _DESCRIPTORS_H_

#include <LUFA/Drivers/USB/USB.h>
#include <avr/pgmspace.h>


typedef struct
{
    USB_Descriptor_Configuration_Header_t Config;

    // Console HID Interface
    USB_Descriptor_Interface_t            Console_Interface;
    USB_HID_Descriptor_HID_t              Console_HID;
    USB_Descriptor_Endpoint_t             Console_INEndpoint;
    USB_Descriptor_Endpoint_t             Console_OUTEndpoint;
} USB_Descriptor_Configuration_t;


/* index of interface */
#define CONSOLE_INTERFACE           0


/* nubmer of interfaces */
#define TOTAL_INTERFACES            1


// Endopoint number and size
#define CONSOLE_IN_EPNUM            1
#define CONSOLE_OUT_EPNUM           1
#define CONSOLE_EPSIZE              32


uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue,
                                    const uint8_t wIndex,
                                    const void** const DescriptorAddress)
                                    ATTR_WARN_UNUSED_RESULT ATTR_NON_NULL_PTR_ARG(3);


/* new API */
#if LUFA_VERSION_INTEGER < 0x140302
    #undef VERSION_BCD
    #define VERSION_BCD(Major, Minor, Revision) \
                                              CPU_TO_LE16( ((Major & 0xFF) << 8) | \
                                                           ((Minor & 0x0F) << 4) | \
                                                           (Revision & 0x0F) )
#endif

#endif
