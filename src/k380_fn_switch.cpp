/*
 *   To compile:
 *
 *   cl k380_fn_on.c /link user32.lib kernel32.lib
 *   
 *   Based on:
 *   http://www.trial-n-error.de/posts/2012/12/31/logitech-k810-keyboard-configurator/
 *   https://github.com/embuc/k480_conf
 *   https://github.com/jergusg/k380-function-keys-conf
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <cstdio>
#include <cstdint>
#include <cerrno>

#include <string>

#include <windows.h>


const uint8_t k380_seq_fkeys_on[] = {0x10, 0xff, 0x0b, 0x1e, 0x00, 0x00, 0x00};
const uint8_t k380_seq_fkeys_off[] = {0x10, 0xff, 0x0b, 0x1e, 0x01, 0x00, 0x00};

int main(int argc, char *argv[]) {
    enum TYPE {
        ON,
        OFF
    };
    TYPE type;

    if (argc > 1) {
        auto type_s = std::string(argv[1]);
        if (type_s == "on") {
            type = ON;
        } else {
            type = OFF;
        }
    } else {
        printf("Usage: %s <on|off>\n", argv[0]);
        exit(0);
    }

    UINT device_num;
    if (GetRawInputDeviceList(nullptr, &device_num, sizeof(RAWINPUTDEVICELIST)) != 0) {
        printf("Can not get input device list\n");
        exit(1);
    }
    auto raw_input_device_list = new RAWINPUTDEVICELIST[device_num];
    if (GetRawInputDeviceList(raw_input_device_list, &device_num, sizeof(RAWINPUTDEVICELIST)) == -1) {
        printf("Can not get input device list\n");
        exit(1);
    }

    printf("Input device num: %d\n", device_num);

    for (decltype(device_num) i = 0; i < device_num; ++i) {
        UINT device_name_size;
        GetRawInputDeviceInfo(
                raw_input_device_list[i].hDevice,
                RIDI_DEVICENAME,
                nullptr,
                &device_name_size);
        auto device_name_c = new char[device_name_size];
        GetRawInputDeviceInfo(
                raw_input_device_list[i].hDevice,
                RIDI_DEVICENAME,
                device_name_c,
                &device_name_size);

        printf("Device #%d: type%d, device_name: %s\n", i, raw_input_device_list[i].dwType, device_name_c);

        std::string device_name(device_name_c);
        if (device_name.find("VID&0002046d_PID&b342&Col06") != std::string::npos) {
            printf("Device %d selected\n", i);

            auto k380 = CreateFile(device_name_c,
                                   GENERIC_WRITE,      //GENERIC_READ |
                                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                                   nullptr,
                                   OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL,
                                   nullptr);

            if (k380 == INVALID_HANDLE_VALUE) {
                fprintf(stderr, "Unable to open device. error_no=%d\n", GetLastError());
            } else {
                DWORD bw;

                auto data = type == ON ? &k380_seq_fkeys_on : &k380_seq_fkeys_off;

                if (WriteFile(k380, *data, 7, &bw, nullptr) == 0) {
                    fprintf(stderr, "error_no=%d\n", GetLastError());
                } else if (bw == 7) {
                    printf("configuration sent.\n");
                } else {
                    printf("write: %d were written instead of %d.\n", bw, 7);
                }
            }
        }

        delete[] device_name_c;
    }

    delete[] raw_input_device_list;
}

