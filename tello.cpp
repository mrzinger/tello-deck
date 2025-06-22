#include "tello.h"
#include "tello_cmd.h"

#include <unistd.h>
#include <string.h>
#include <thread>

uint8_t table8[] = {
    0x00, 0x5e, 0xbc, 0xe2, 0x61, 0x3f, 0xdd, 0x83, 0xc2, 0x9c, 0x7e, 0x20, 0xa3, 0xfd, 0x1f, 0x41,
    0x9d, 0xc3, 0x21, 0x7f, 0xfc, 0xa2, 0x40, 0x1e, 0x5f, 0x01, 0xe3, 0xbd, 0x3e, 0x60, 0x82, 0xdc,
    0x23, 0x7d, 0x9f, 0xc1, 0x42, 0x1c, 0xfe, 0xa0, 0xe1, 0xbf, 0x5d, 0x03, 0x80, 0xde, 0x3c, 0x62,
    0xbe, 0xe0, 0x02, 0x5c, 0xdf, 0x81, 0x63, 0x3d, 0x7c, 0x22, 0xc0, 0x9e, 0x1d, 0x43, 0xa1, 0xff,
    0x46, 0x18, 0xfa, 0xa4, 0x27, 0x79, 0x9b, 0xc5, 0x84, 0xda, 0x38, 0x66, 0xe5, 0xbb, 0x59, 0x07,
    0xdb, 0x85, 0x67, 0x39, 0xba, 0xe4, 0x06, 0x58, 0x19, 0x47, 0xa5, 0xfb, 0x78, 0x26, 0xc4, 0x9a,
    0x65, 0x3b, 0xd9, 0x87, 0x04, 0x5a, 0xb8, 0xe6, 0xa7, 0xf9, 0x1b, 0x45, 0xc6, 0x98, 0x7a, 0x24,
    0xf8, 0xa6, 0x44, 0x1a, 0x99, 0xc7, 0x25, 0x7b, 0x3a, 0x64, 0x86, 0xd8, 0x5b, 0x05, 0xe7, 0xb9,
    0x8c, 0xd2, 0x30, 0x6e, 0xed, 0xb3, 0x51, 0x0f, 0x4e, 0x10, 0xf2, 0xac, 0x2f, 0x71, 0x93, 0xcd,
    0x11, 0x4f, 0xad, 0xf3, 0x70, 0x2e, 0xcc, 0x92, 0xd3, 0x8d, 0x6f, 0x31, 0xb2, 0xec, 0x0e, 0x50,
    0xaf, 0xf1, 0x13, 0x4d, 0xce, 0x90, 0x72, 0x2c, 0x6d, 0x33, 0xd1, 0x8f, 0x0c, 0x52, 0xb0, 0xee,
    0x32, 0x6c, 0x8e, 0xd0, 0x53, 0x0d, 0xef, 0xb1, 0xf0, 0xae, 0x4c, 0x12, 0x91, 0xcf, 0x2d, 0x73,
    0xca, 0x94, 0x76, 0x28, 0xab, 0xf5, 0x17, 0x49, 0x08, 0x56, 0xb4, 0xea, 0x69, 0x37, 0xd5, 0x8b,
    0x57, 0x09, 0xeb, 0xb5, 0x36, 0x68, 0x8a, 0xd4, 0x95, 0xcb, 0x29, 0x77, 0xf4, 0xaa, 0x48, 0x16,
    0xe9, 0xb7, 0x55, 0x0b, 0x88, 0xd6, 0x34, 0x6a, 0x2b, 0x75, 0x97, 0xc9, 0x4a, 0x14, 0xf6, 0xa8,
    0x74, 0x2a, 0xc8, 0x96, 0x15, 0x4b, 0xa9, 0xf7, 0xb6, 0xe8, 0x0a, 0x54, 0xd7, 0x89, 0x6b, 0x35,
};

uint16_t table16[] = {
    0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf, 0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
    0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e, 0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
    0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd, 0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
    0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c, 0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
    0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb, 0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
    0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a, 0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
    0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9, 0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
    0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738, 0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
    0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7, 0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
    0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036, 0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
    0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5, 0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
    0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134, 0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
    0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3, 0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
    0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232, 0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
    0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1, 0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
    0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330, 0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78,
};

int Tello::crc_8(uint8_t message[], int length)
{
	int i = 0;
	int crc = 0x77;
	for (i = 0; i < length; i++) crc = table8[(crc ^ message[i]) & 0xFF];
	return crc;
}

int Tello::crc_16(uint8_t message[], int length)
{
	int i = 0;
	int crc = 0x3692;
	for (i = 0; i < length; i++) crc = table16[(crc ^ message[i]) & 0xFF] ^ (crc >> 8);
	return crc;
}

void Tello::send_package(int type, int id, int sequence, int* data, int datasize)
{
	int packagesize = 11 + datasize;
	uint8_t package[packagesize];
	int start = HEADER;
	int size = packagesize * 8;
	memcpy(&package[0], &start, 1);
	memcpy(&package[1], &size, 2);
	int crc8 = crc_8(package, 3);
	memcpy(&package[3], &crc8, 1);
	memcpy(&package[4], &type, 1);
	memcpy(&package[5], &id, 2);
	memcpy(&package[7], &sequence, 2);
	for (int i = 0; i < datasize; i++) memcpy(&package[9+i], &data[i], 1);
	int crc16 = crc_16(package, 9+datasize);
	memcpy(&package[9+datasize], &crc16, 2);
        sendto(data_socket, package, packagesize, 0, (struct sockaddr *)&address, sizeof(address));
}


void *Tello::poll_thread(void *ptr)
{
  Tello *tello = (Tello *)ptr;
	while (tello->data_socket > 0) {
		long a1 = tello->speed_mode;
		long a2 = tello->left_x * 660 + 1024;
		long a3 = tello->left_y * 660 + 1024;
		long a4 = tello->right_y * 660 + 1024;
		long a5 = tello->right_x * 660 + 1024;
		long packedAxis = (a1 << 44) | (a2 << 33) | (a3 << 22) | (a4 << 11) | a5;
		struct timespec ts;
	    	clock_gettime(CLOCK_REALTIME, &ts);
		struct tm *lt = localtime(&ts.tv_sec);
		int msec = ts.tv_nsec/1000000;
		int data[] = {
			packedAxis & 0xFF, packedAxis >> 8 & 0xFF, packedAxis >> 16 & 0xFF,
			packedAxis >> 24 & 0xFF, packedAxis >> 32 & 0xFF, packedAxis >> 40 & 0xFF,
			lt->tm_hour, lt->tm_min, lt->tm_sec, msec & 0xFF, msec >> 8 & 0xFF
		};
                tello->send_package(96, SET_STICKS, 0, data, 11);
		usleep(20000);
        }
}

void *Tello::data_thread(void *ptr)
{
  Tello *tello = (Tello *)ptr;
	uint8_t package[1012];
	while (tello->data_socket > 0) {
		recv(tello->data_socket, package, 1012, 0);
		int size = (package[1] | package[2] << 8) / 8;
		int crc8 = package[3];
		if (crc8 != crc_8(package, 3)) continue;
		int id = package[5] | package[6] << 8;
		int datasize = size - 11;	
		int crc16 = package[9+datasize] | package[10+datasize] << 8;
		if (crc16 != crc_16(package, 9+datasize)) continue;
		switch (id) {
			case WIFI_STRENGTH: {
				tello->wifi_strength = package[9];
				tello->wifi_disturb = package[10];
				if (tello->data_callback) (*tello->data_callback)(id);
				continue;
			}
			case LIGHT_STRENGTH: {
				tello->light_strength = package[9];
				if (tello->data_callback) (*tello->data_callback)(id);
				continue;
			}
			case SET_DATE_TIME: {
				struct timespec ts;
			    	clock_gettime(CLOCK_REALTIME, &ts);
				struct tm *lt = localtime(&ts.tv_sec);
				int msec = ts.tv_nsec/1000000;
				int senddata[] = {
					lt->tm_year & 0xFF, lt->tm_year >> 8 & 0xFF,
					lt->tm_mon & 0xFF, lt->tm_mon >> 8 & 0xFF,
					lt->tm_mday & 0xFF, lt->tm_mday >> 8 & 0xFF,
					lt->tm_hour & 0xFF, lt->tm_hour >> 8 & 0xFF,
					lt->tm_min & 0xFF, lt->tm_min >> 8 & 0xFF,
					lt->tm_sec & 0xFF, lt->tm_sec >> 8 & 0xFF,
					msec & 0xFF, msec >> 8 & 0xFF
				};
                                tello->send_package(80, SET_DATE_TIME, tello->sequence++, senddata, 12);
				continue;
			}
			case FLIGHT_STATUS: {
				memcpy(&tello->height, &package[9], 2);
				memcpy(&tello->north_speed, &package[11], 2);
				memcpy(&tello->east_speed, &package[13], 2);
				memcpy(&tello->ground_speed, &package[15], 2);
				memcpy(&tello->fly_time, &package[17], 2);
				tello->imu_state = package[19] >> 0 & 1;
				tello->pressure_state = package[19] >> 1 & 1;
				tello->visual_state = package[19] >> 2 & 1;
				tello->power_state = package[19] >> 3 & 1;
				tello->battery_state = package[19] >> 4 & 1;
				tello->gravity_state = package[19] >> 5 & 1;
				tello->wind_state = package[19] >> 7 & 1;
				tello->imu_calibration_state = package[20];
				tello->battery_percentage = package[21];
				memcpy(&tello->fly_time_left, &package[22], 2);
				memcpy(&tello->battery_left, &package[24], 2);
				tello->sky = package[26] >> 0 & 1;
				tello->ground = package[26] >> 1 & 1;
				tello->open = package[26] >> 2 & 1;
				tello->hover = package[26] >> 3 & 1;
				tello->outage_recording = package[26] >> 4 & 1;
				tello->battery_low = package[26] >> 5 & 1;
				tello->battery_lower = package[26] >> 6 & 1;
				tello->factory_mode = package[26] >> 7 & 1;
				tello->fly_mode = package[27];
				tello->throw_fly_timer = package[28];
				tello->camera_state = package[29];
				tello->electrical_machinery_state = package[30];
				tello->front_in = package[31] >> 0 & 1;
				tello->front_out = package[31] >> 1 & 1;
				tello->front_lsc = package[31] >> 2 & 1;
				tello->temperature_height = package[32] >> 0 & 1;
				if (tello->data_callback) (*tello->data_callback)(id);
				continue;
			}
			case LOG_HEADER: {
				int senddata[] = {0, package[9], package[10]};
                                tello->send_package(80, LOG_HEADER, tello->sequence++, senddata, 3);
				continue;
			}
			case LOG_DATA: {
				int position = 10;
				while (position < size - 2) {
					if (package[position + 0] != 85) break;
					int chunkSize = package[position + 1];
					if (package[position + 2] != 0) break;
					int crc8 = package[position + 3];
					if (crc8 != crc_8(&package[position], 3)) break;
					int id = package[position + 4] | package[position + 5] << 8;
                   			uint8_t decrypt = package[position + 6];
					switch (id) {
						case 29: {
							for (int i = 0; i < chunkSize; i++) package[position + i] = package[position + i] ^ decrypt;
							memcpy(&tello->velocity_z, &package[position + 12], 2);
							memcpy(&tello->velocity_x, &package[position + 14], 2);
							memcpy(&tello->velocity_y, &package[position + 16], 2);
							memcpy(&tello->position_z, &package[position + 18], 4);
							memcpy(&tello->position_x, &package[position + 22], 4);
							memcpy(&tello->position_y, &package[position + 26], 4);
							memcpy(&tello->position_uncertainty, &package[position + 30], 4);
							if (tello->data_callback) (*tello->data_callback)(id);
							break;
						}
						case 2048: {
							for (int i = 0; i < chunkSize; i++) package[position + i] = package[position + i] ^ decrypt;
							memcpy(&tello->rotation_w, &package[position + 58], 4);
							memcpy(&tello->rotation_z, &package[position + 62], 4);
							memcpy(&tello->rotation_x, &package[position + 66], 4);
							memcpy(&tello->rotation_y, &package[position + 70], 4);
							memcpy(&tello->relative_velocity_z, &package[position + 86], 4);
							memcpy(&tello->relative_velocity_x, &package[position + 90], 4);
							memcpy(&tello->relative_velocity_y, &package[position + 94], 4);
							if (tello->data_callback) (*tello->data_callback)(id);
							break;
						}
					}
					position += chunkSize;
				}
				continue;
			}
		}		
        }
}


void *Tello::camera_thread(void *ptr)
{
  Tello *tello = (Tello *)ptr;
	uint8_t package[1460];
	while (tello->camera_socket > 0) {
		int size = recv(tello->camera_socket, package, 1460, 0);
		if (tello->camera_callback) (*tello->camera_callback)(package, size);
        }
}

void Tello::disconnect()
{
  tello->data_socket.close();
  tello->camera_socket.close();
}

int Tello::connect(int camera_port, int timeout)
{
  tello->data_socket.reset(socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP));

  address.sin_family = AF_INET;
  address.sin_port = htons(8889);
  inet_pton(AF_INET, "192.168.10.1", &(address.sin_addr));

	char req_package[12];
	memcpy(&req_package[0], "conn_req:", 9);
	memcpy(&req_package[9], &camera_port, 2);
	req_package[11] = '\0';

	char ack_package[12];
	memcpy(&ack_package[0], "conn_ack:", 9);
	memcpy(&ack_package[9], &camera_port, 2);
	ack_package[11] = '\0';
	
        sendto(data_socket, req_package, 11, 0, (struct sockaddr *)&address, sizeof(address));

	struct timeval tv; tv.tv_sec = timeout; tv.tv_usec = 0;
        setsockopt(data_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        if (recv(data_socket, req_package, 11, 0) < 0) { disconnect(); return -1; }
        if (strcmp(req_package, ack_package) != 0) { disconnect(); return -1; }
	
	tv.tv_sec = 0; tv.tv_usec = 0;
        setsockopt(data_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        tello->camera_socket.reset(socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP));


	struct sockaddr_in camera_address;
	memset(&camera_address, '0', sizeof(camera_address));
	camera_address.sin_family = AF_INET;
	camera_address.sin_port = htons(camera_port);
	camera_address.sin_addr.s_addr = htonl(INADDR_ANY);
  bind(camera_socket, (struct sockaddr*)&camera_address, sizeof(camera_address));


  std::thread(poll_thread, tello).detach();
  std::thread(data_thread, tello).detach();
  std::thread(camera_thread, tello).detach();

	return 0;
}

void Tello::request_iframe()
{
        send_package(96, REQUEST_VIDEO_START, 0, NULL, 0);
}

void Tello::camera_mode(int value)
{
        int data[] = {value};
        send_package(104, SET_VIDEO_ASPECT, sequence++, data, 1);
}

void Tello::takeoff()
{
        send_package(104, TAKE_OFF, sequence++, NULL, 0);
}

void Tello::land()
{
        int data[] = {0};
        send_package(104, LAND, sequence++, data, 1);
}

