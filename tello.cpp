#include "tello.h"
#include "tello_cmd.h"
#include "crc_utils.h"
#include <unistd.h>
#include <string.h>
#include <thread>

void Tello::send_package(int type, int id, int sequence, int *data, int datasize)
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
	for (int i = 0; i < datasize; i++)
		memcpy(&package[9 + i], &data[i], 1);
	int crc16 = crc_16(package, 9 + datasize);
	memcpy(&package[9 + datasize], &crc16, 2);
	sendto(data_socket, package, packagesize, 0, (struct sockaddr *)&address, sizeof(address));
}

void Tello::poll_thread(Tello *tello)
{
	while (tello->data_socket > 0)
	{
		long a1 = tello->speed_mode;
		long a2 = tello->left_x * 660 + 1024;
		long a3 = tello->left_y * 660 + 1024;
		long a4 = tello->right_y * 660 + 1024;
		long a5 = tello->right_x * 660 + 1024;
		long packedAxis = (a1 << 44) | (a2 << 33) | (a3 << 22) | (a4 << 11) | a5;
		struct timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);
		struct tm *lt = localtime(&ts.tv_sec);
		int msec = ts.tv_nsec / 1000000;
		int data[] = {
			packedAxis & 0xFF, packedAxis >> 8 & 0xFF, packedAxis >> 16 & 0xFF,
			packedAxis >> 24 & 0xFF, packedAxis >> 32 & 0xFF, packedAxis >> 40 & 0xFF,
			lt->tm_hour, lt->tm_min, lt->tm_sec, msec & 0xFF, msec >> 8 & 0xFF};
		tello->send_package(96, SET_STICKS, 0, data, 11);
		usleep(20000);
	}
}

void Tello::data_thread(Tello *tello)
{
	uint8_t package[1012];
	while (tello->data_socket > 0)
	{
		recv(tello->data_socket, package, 1012, 0);
		int size = (package[1] | package[2] << 8) / 8;
		int crc8 = package[3];
		if (crc8 != crc_8(package, 3))
			continue;
		int id = package[5] | package[6] << 8;
		int datasize = size - 11;
		int crc16 = package[9 + datasize] | package[10 + datasize] << 8;
		if (crc16 != crc_16(package, 9 + datasize))
			continue;
		switch (id)
		{
		case WIFI_STRENGTH:
		{
			tello->wifi_strength = package[9];
			tello->wifi_disturb = package[10];
			if (tello->data_callback)
				(*tello->data_callback)(id);
			continue;
		}
		case LIGHT_STRENGTH:
		{
			tello->light_strength = package[9];
			if (tello->data_callback)
				(*tello->data_callback)(id);
			continue;
		}
		case SET_DATE_TIME:
		{
			struct timespec ts;
			clock_gettime(CLOCK_REALTIME, &ts);
			struct tm *lt = localtime(&ts.tv_sec);
			int msec = ts.tv_nsec / 1000000;
			int senddata[] = {
				lt->tm_year & 0xFF, lt->tm_year >> 8 & 0xFF,
				lt->tm_mon & 0xFF, lt->tm_mon >> 8 & 0xFF,
				lt->tm_mday & 0xFF, lt->tm_mday >> 8 & 0xFF,
				lt->tm_hour & 0xFF, lt->tm_hour >> 8 & 0xFF,
				lt->tm_min & 0xFF, lt->tm_min >> 8 & 0xFF,
				lt->tm_sec & 0xFF, lt->tm_sec >> 8 & 0xFF,
				msec & 0xFF, msec >> 8 & 0xFF};
			tello->send_package(80, SET_DATE_TIME, tello->sequence++, senddata, 12);
			continue;
		}
		case FLIGHT_STATUS:
		{
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
			if (tello->data_callback)
				(*tello->data_callback)(id);
			continue;
		}
		case LOG_HEADER:
		{
			int senddata[] = {0, package[9], package[10]};
			tello->send_package(80, LOG_HEADER, tello->sequence++, senddata, 3);
			continue;
		}
		case LOG_DATA:
		{
			int position = 10;
			while (position < size - 2)
			{
				if (package[position + 0] != 85)
					break;
				int chunkSize = package[position + 1];
				if (package[position + 2] != 0)
					break;
				int crc8 = package[position + 3];
				if (crc8 != crc_8(&package[position], 3))
					break;
				int id = package[position + 4] | package[position + 5] << 8;
				uint8_t decrypt = package[position + 6];
				switch (id)
				{
				case 29:
				{
					for (int i = 0; i < chunkSize; i++)
						package[position + i] = package[position + i] ^ decrypt;
					memcpy(&tello->velocity_z, &package[position + 12], 2);
					memcpy(&tello->velocity_x, &package[position + 14], 2);
					memcpy(&tello->velocity_y, &package[position + 16], 2);
					memcpy(&tello->position_z, &package[position + 18], 4);
					memcpy(&tello->position_x, &package[position + 22], 4);
					memcpy(&tello->position_y, &package[position + 26], 4);
					memcpy(&tello->position_uncertainty, &package[position + 30], 4);
					if (tello->data_callback)
						(*tello->data_callback)(id);
					break;
				}
				case 2048:
				{
					for (int i = 0; i < chunkSize; i++)
						package[position + i] = package[position + i] ^ decrypt;
					memcpy(&tello->rotation_w, &package[position + 58], 4);
					memcpy(&tello->rotation_z, &package[position + 62], 4);
					memcpy(&tello->rotation_x, &package[position + 66], 4);
					memcpy(&tello->rotation_y, &package[position + 70], 4);
					memcpy(&tello->relative_velocity_z, &package[position + 86], 4);
					memcpy(&tello->relative_velocity_x, &package[position + 90], 4);
					memcpy(&tello->relative_velocity_y, &package[position + 94], 4);
					if (tello->data_callback)
						(*tello->data_callback)(id);
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

void Tello::camera_thread(Tello * tello)
{
	uint8_t package[1460];
	while (tello->camera_socket > 0)
	{
		int size = recv(tello->camera_socket, package, 1460, 0);
		if (tello->camera_callback)
			(*tello->camera_callback)(package, size);
	}
}

void Tello::disconnect()
{
	this->data_socket.close();
	this->camera_socket.close();
}

int Tello::connect(int camera_port, int timeout)
{
	data_socket.reset(socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP));

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

	struct timeval tv;
	tv.tv_sec = timeout;
	tv.tv_usec = 0;
	setsockopt(data_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

	if (recv(data_socket, req_package, 11, 0) < 0)
	{
		disconnect();
		return -1;
	}
	if (strcmp(req_package, ack_package) != 0)
	{
		disconnect();
		return -1;
	}

	tv.tv_sec = 0;
	tv.tv_usec = 0;
	setsockopt(data_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)); //Update timeout to 0 for non-blocking

	camera_socket.reset(socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP));

	struct sockaddr_in camera_address;
	memset(&camera_address, '0', sizeof(camera_address));
	camera_address.sin_family = AF_INET;
	camera_address.sin_port = htons(camera_port);
	camera_address.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(camera_socket, (struct sockaddr *)&camera_address, sizeof(camera_address));

	std::thread(poll_thread, this).detach();
	std::thread(data_thread, this).detach();
	std::thread(camera_thread, this).detach();

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
