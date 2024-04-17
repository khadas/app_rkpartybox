/*
 * Copyright 2023 Rockchip Electronics Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <errno.h>
#include <stdbool.h>
#include <sys/socket.h>
#include "pbox_light_effect.h"
#include "pbox_socket.h"
#include "pbox_socketpair.h"
#include "pbox_common.h"
#include "pthread.h"
#include "pbox_ledctrl.h"
#include "pbox_led_cjson.h"
#include "os_minor_type.h"

struct led_effect *leffect;
struct led_effect *foreground_leffect;
struct effect_calcule_data *cal_data;
struct light_effect_ctrl * ctrl = NULL;

int foreground_leffect_job;

#define msleep(x) usleep(x * 1000)
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/* this array holds the RGB values to represent 
 * a color wheel using 256 steps on each emitter
 * 256^3 = 16777216 colors
 * It was taken from http://eliaselectronics.com/
 */
uint8_t color_wheel[766][3] = {
	{ 255, 0, 0 }, { 254, 1, 0 }, { 253, 2, 0 }, {252, 3, 0 }, { 251, 4, 0 }, { 250, 5, 0 },
	{ 249, 6, 0 }, { 248, 7, 0 },{ 247, 8, 0 }, { 246, 9, 0 }, { 245, 10, 0 }, { 244, 11, 0 },
	{ 243, 12, 0 }, { 242, 13, 0 }, { 241, 14, 0 }, { 240, 15, 0 }, { 239, 16,0 }, { 238, 17, 0 },
	{ 237, 18, 0 }, { 236, 19, 0 }, { 235, 20,0 }, { 234, 21, 0 }, { 233, 22, 0 }, { 232, 23, 0 },
	{ 231, 24, 0 }, { 230, 25, 0 }, { 229, 26, 0 }, { 228, 27, 0 }, { 227, 28, 0 }, { 226, 29, 0 },
	{ 225, 30, 0 }, { 224, 31, 0 }, { 223, 32, 0 }, { 222, 33, 0 }, { 221, 34, 0 }, { 220, 35, 0 },
	{ 219, 36, 0 }, { 218, 37, 0 }, { 217, 38, 0 }, { 216, 39, 0 }, { 215, 40, 0 }, { 214, 41, 0 },
	{ 213, 42, 0 }, { 212, 43, 0 }, { 211, 44, 0 }, { 210, 45, 0 }, { 209, 46, 0 }, { 208, 47, 0 },
	{ 207, 48, 0 }, { 206, 49, 0 }, { 205, 50, 0 }, { 204, 51, 0 }, { 203, 52, 0 }, { 202, 53, 0 },
	{ 201, 54, 0 }, { 200, 55, 0 }, { 199, 56, 0 }, { 198, 57, 0 }, { 197, 58, 0 }, { 196, 59, 0 },
	{ 195, 60, 0 }, { 194, 61, 0 }, { 193, 62, 0 }, { 192, 63, 0 }, { 191, 64, 0 }, { 190, 65, 0 },
	{ 189, 66, 0 }, { 188, 67, 0 }, { 187, 68, 0 }, { 186, 69, 0 }, { 185, 70, 0 }, { 184, 71, 0 },
	{ 183, 72, 0 }, { 182, 73, 0 }, { 181, 74, 0 }, { 180, 75, 0 }, { 179, 76, 0 }, { 178, 77, 0 },
	{ 177, 78, 0 }, { 176, 79, 0 }, { 175, 80, 0 }, { 174, 81, 0 }, { 173, 82, 0 }, { 172, 83, 0 },
	{ 171, 84, 0 }, { 170, 85, 0 }, { 169, 86, 0 }, { 168, 87, 0 }, { 167, 88, 0 }, { 166, 89, 0 },
	{ 165, 90, 0 }, { 164, 91, 0 }, { 163, 92, 0 }, { 162, 93, 0 }, { 161, 94, 0 }, { 160, 95, 0 },
	{ 159, 96, 0 }, { 158, 97, 0 }, { 157, 98, 0 }, { 156, 99, 0 }, { 155, 100, 0 }, { 154, 101, 0 },
	{ 153, 102, 0 }, { 152, 103, 0 }, { 151, 104, 0 }, { 150, 105, 0 }, { 149, 106, 0 }, { 148, 107, 0 },
	{ 147, 108, 0 }, { 146, 109, 0 }, { 145, 110, 0 }, { 144, 111, 0 }, { 143, 112, 0 }, { 142, 113, 0 },
	{ 141, 114, 0 }, { 140, 115, 0 }, { 139, 116, 0 }, { 138, 117, 0 }, { 137, 118, 0 }, { 136, 119, 0 },
	{ 135, 120, 0 }, { 134, 121, 0 }, { 133, 122, 0 }, { 132, 123, 0 }, { 131, 124, 0 }, { 130, 125, 0 },
	{ 129, 126, 0 }, { 128, 127, 0 }, { 127, 128, 0 }, { 126, 129, 0 }, { 125, 130, 0 }, { 124, 131, 0 },
	{ 123, 132, 0 }, { 122, 133, 0 }, { 121, 134, 0 }, { 120, 135, 0 }, { 119, 136, 0 }, { 118, 137, 0 },
	{ 117, 138, 0 }, { 116, 139, 0 }, { 115, 140, 0 }, { 114, 141, 0 }, { 113, 142, 0 }, { 112, 143, 0 },
	{ 111, 144, 0 }, { 110, 145, 0 }, { 109, 146, 0 }, { 108, 147, 0 }, { 107, 148, 0 }, { 106, 149, 0 },
	{ 105, 150, 0 }, { 104, 151, 0 }, { 103, 152, 0 }, { 102, 153, 0 }, { 101, 154, 0 }, { 100, 155, 0 },
	{ 99, 156, 0 }, { 98, 157, 0 }, { 97, 158, 0 }, { 96, 159, 0 }, { 95, 160, 0 }, { 94, 161, 0 },
	{ 93, 162, 0 }, { 92, 163, 0 }, { 91, 164, 0 }, { 90, 165, 0 }, { 89, 166, 0 }, { 88, 167, 0 },
	{ 87, 168, 0 }, { 86, 169, 0 }, { 85, 170, 0 }, { 84, 171, 0 }, { 83, 172, 0 }, { 82, 173, 0 },
	{ 81, 174, 0 }, { 80, 175, 0 }, { 79, 176, 0 }, { 78, 177, 0 }, { 77, 178, 0 }, { 76, 179, 0 },
	{ 75, 180, 0 }, { 74, 181, 0 }, { 73, 182, 0 }, { 72, 183, 0 }, { 71, 184, 0 }, { 70, 185, 0 },
	{ 69, 186, 0 }, { 68, 187, 0 }, { 67, 188, 0 }, { 66, 189, 0 }, { 65, 190, 0 }, { 64, 191, 0 },
	{ 63, 192, 0 }, { 62, 193, 0 }, { 61, 194, 0 }, { 60, 195, 0 }, { 59, 196, 0 }, { 58, 197, 0 },
	{ 57, 198, 0 }, { 56, 199, 0 }, { 55, 200, 0 }, { 54, 201, 0 }, { 53, 202, 0 }, { 52, 203, 0 },
	{ 51, 204, 0 }, { 50, 205, 0 }, { 49, 206, 0 }, { 48, 207, 0 }, { 47, 208, 0 }, { 46, 209, 0 },
	{ 45, 210, 0 }, { 44, 211, 0 }, { 43, 212, 0 }, { 42, 213, 0 }, { 41, 214, 0 }, { 40, 215, 0 },
	{ 39, 216, 0 }, { 38, 217, 0 }, { 37, 218, 0 }, { 36, 219, 0 }, { 35, 220, 0 }, { 34, 221, 0 },
	{ 33, 222, 0 }, { 32, 223, 0 }, { 31, 224, 0 }, { 30, 225, 0 }, { 29, 226, 0 }, { 28, 227, 0 },
	{ 27, 228, 0 }, { 26, 229, 0 }, { 25, 230, 0 }, { 24, 231, 0 }, { 23, 232, 0 }, { 22, 233, 0 },
	{ 21, 234, 0 }, { 20, 235, 0 }, { 19, 236, 0 }, { 18, 237, 0 }, { 17, 238, 0 }, { 16, 239, 0 },
	{ 15, 240, 0 }, { 14, 241, 0 }, { 13, 242, 0 }, { 12, 243, 0 }, { 11, 244, 0 }, { 10, 245, 0 },
	{ 9, 246, 0 }, { 8, 247, 0 }, { 7, 248, 0 }, { 6, 249, 0 }, { 5, 250, 0 }, { 4, 251, 0 },
	{ 3, 252, 0 }, { 2, 253, 0 }, { 1, 254, 0 }, { 0, 255, 0 }, { 0, 254, 1 }, { 0, 253, 2 },
	{ 0, 252, 3 }, { 0, 251, 4 }, { 0, 250, 5 }, { 0, 249, 6 }, { 0, 248, 7 }, { 0, 247, 8 },
	{ 0, 246, 9 }, { 0, 245, 10 }, { 0, 244, 11 }, { 0, 243, 12 }, { 0, 242, 13 }, { 0, 241, 14 },
	{ 0, 240, 15 }, { 0, 239, 16 }, { 0, 238, 17 }, { 0, 237, 18 }, { 0, 236, 19 }, { 0, 235, 20 },
	{ 0, 234, 21 }, { 0, 233, 22 }, { 0, 232, 23 }, { 0, 231, 24 }, { 0, 230, 25 }, { 0, 229, 26 },
	{ 0, 228, 27 }, { 0, 227, 28 }, { 0, 226, 29 }, { 0, 225, 30 }, { 0, 224, 31 }, { 0, 223, 32 },
	{ 0, 222, 33 }, { 0, 221, 34 }, { 0, 220, 35 }, { 0, 219, 36 }, { 0, 218, 37 }, { 0, 217, 38 },
	{ 0, 216, 39 }, { 0, 215, 40 }, { 0, 214, 41 }, { 0, 213, 42 }, { 0, 212, 43 }, { 0, 211, 44 },
	{ 0, 210, 45 }, { 0, 209, 46 }, { 0, 208, 47 }, { 0, 207, 48 }, { 0, 206, 49 }, { 0, 205, 50 },
	{ 0, 204, 51 }, { 0, 203, 52 }, { 0, 202, 53 }, { 0, 201, 54 }, { 0, 200, 55 }, { 0, 199, 56 },
	{ 0, 198, 57 }, { 0, 197, 58 }, { 0, 196, 59 }, { 0, 195, 60 }, { 0, 194, 61 }, { 0, 193, 62 },
	{ 0, 192, 63 }, { 0, 191, 64 }, { 0, 190, 65 }, { 0, 189, 66 }, { 0, 188, 67 }, { 0, 187, 68 },
	{ 0, 186, 69 }, { 0, 185, 70 }, { 0, 184, 71 }, { 0, 183, 72 }, { 0, 182, 73 }, { 0, 181, 74 },
	{ 0, 180, 75 }, { 0, 179, 76 }, { 0, 178, 77 }, { 0, 177, 78 }, { 0, 176, 79 }, { 0, 175, 80 },
	{ 0, 174, 81 }, { 0, 173, 82 }, { 0, 172, 83 }, { 0, 171, 84 }, { 0, 170, 85 }, { 0, 169, 86 },
	{ 0, 168, 87 }, { 0, 167, 88 }, { 0, 166, 89 }, { 0, 165, 90 }, { 0, 164, 91 }, { 0, 163, 92 },
	{ 0, 162, 93 }, { 0, 161, 94 }, { 0, 160, 95 }, { 0, 159, 96 }, { 0, 158, 97 }, { 0, 157, 98 },
	{ 0, 156, 99 }, { 0, 155, 100 }, { 0, 154, 101 }, { 0, 153, 102 }, { 0, 152, 103 }, { 0, 151, 104 },
	{ 0, 150, 105 }, { 0, 149, 106 }, { 0, 148, 107 }, { 0, 147, 108 }, { 0, 146, 109 }, { 0, 145, 110 },
	{ 0, 144, 111 }, { 0, 143, 112 }, { 0, 142, 113 }, { 0, 141, 114 }, { 0, 140, 115 }, { 0, 139, 116 },
	{ 0, 138, 117 }, { 0, 137, 118 }, { 0, 136, 119 }, { 0, 135, 120 }, { 0, 134, 121 }, { 0, 133, 122 },
	{ 0, 132, 123 }, { 0, 131, 124 }, { 0, 130, 125 }, { 0, 129, 126 }, { 0, 128, 127 }, { 0, 127, 128 },
	{0, 126, 129 }, { 0, 125, 130 }, { 0, 124, 131 }, { 0, 123, 132 }, { 0, 122, 133 }, { 0, 121, 134 },
	{ 0, 120, 135 }, { 0, 119, 136 }, { 0, 118, 137 }, { 0, 117, 138 }, { 0, 116, 139 }, { 0, 115, 140 },
	{ 0, 114, 141 }, { 0, 113, 142 }, { 0, 112, 143 }, { 0, 111, 144 }, { 0, 110, 145 }, { 0, 109, 146 },
	{ 0, 108, 147 }, { 0, 107, 148 }, { 0, 106, 149 }, { 0, 105, 150 }, { 0, 104, 151 }, { 0, 103, 152 },
	{ 0, 102, 153 }, { 0, 101, 154 }, { 0, 100, 155 }, { 0, 99, 156 }, { 0, 98, 157 }, { 0, 97, 158 },
	{ 0, 96, 159 }, { 0, 95, 160 }, { 0, 94, 161 }, { 0, 93, 162 }, { 0, 92, 163 }, { 0, 91, 164 },
	{ 0, 90, 165 }, { 0, 89, 166 }, { 0, 88, 167 }, { 0, 87, 168 }, { 0, 86, 169 }, { 0, 85, 170 },
	{ 0, 84, 171 }, { 0, 83, 172 }, { 0, 82, 173 }, { 0, 81, 174 }, { 0, 80, 175 }, { 0, 79, 176 },
	{ 0, 78, 177 }, { 0, 77, 178 }, { 0, 76, 179 }, { 0, 75, 180 }, { 0, 74, 181 }, { 0, 73, 182 },
	{ 0, 72, 183 }, { 0, 71, 184 }, { 0, 70, 185 }, { 0, 69, 186 }, { 0, 68, 187 }, { 0, 67, 188 },
	{ 0, 66, 189 }, { 0, 65, 190 }, { 0, 64, 191 }, { 0, 63, 192 }, { 0, 62, 193 }, { 0, 61, 194 },
	{ 0, 60, 195 }, { 0, 59, 196 }, { 0, 58, 197 }, { 0, 57, 198 }, { 0, 56, 199 }, { 0, 55, 200 },
	{ 0, 54, 201 }, { 0, 53, 202 }, { 0, 52, 203 }, { 0, 51, 204 }, { 0, 50, 205 }, { 0, 49, 206 },
	{ 0, 48, 207 }, { 0, 47, 208 }, { 0, 46, 209 }, { 0, 45, 210 }, { 0, 44, 211 }, { 0, 43, 212 },
	{ 0, 42, 213 }, { 0, 41, 214 }, { 0, 40, 215 }, { 0, 39, 216 }, { 0, 38, 217 }, { 0, 37, 218 },
	{ 0, 36, 219 }, { 0, 35, 220 }, { 0, 34, 221 }, { 0, 33, 222 }, { 0, 32, 223 }, { 0, 31, 224 },
	{ 0, 30, 225 }, { 0, 29, 226 }, { 0, 28, 227 }, { 0, 27, 228 }, { 0, 26, 229 }, { 0, 25, 230 },
	{ 0, 24, 231 }, { 0, 23, 232 }, { 0, 22, 233 }, { 0, 21, 234 }, { 0, 20, 235 }, { 0, 19, 236 },
	{ 0, 18, 237 }, { 0, 17, 238 }, { 0, 16, 239 }, { 0, 15, 240 }, { 0, 14, 241 }, { 0, 13, 242 },
	{ 0, 12, 243 }, { 0, 11, 244 }, { 0, 10, 245 }, { 0, 9, 246 }, { 0, 8, 247 }, { 0, 7, 248 },
	{ 0, 6, 249 }, { 0, 5, 250 }, { 0, 4, 251 }, { 0, 3, 252 }, { 0, 2, 253 }, { 0, 1, 254 },
	{ 0, 0, 255 }, { 1, 0, 254 }, { 2, 0, 253 }, { 3, 0, 252 }, { 4, 0, 251 }, { 5, 0, 250 },
	{ 6, 0, 249 }, { 7, 0, 248 }, { 8, 0, 247 }, { 9, 0, 246 }, { 10, 0, 245 }, { 11, 0, 244 },
	{ 12, 0, 243 }, { 13, 0, 242 }, { 14, 0, 241 }, { 15, 0, 240 }, { 16, 0, 239 }, { 17, 0, 238 },
	{ 18, 0, 237 }, { 19, 0, 236 }, { 20, 0, 235 }, { 21, 0, 234 }, { 22, 0, 233 }, { 23, 0, 232 },
	{ 24, 0, 231 }, { 25, 0, 230 }, { 26, 0, 229 }, { 27, 0, 228 }, { 28, 0, 227 }, { 29, 0, 226 },
	{ 30, 0, 225 }, { 31, 0, 224 }, { 32, 0, 223 }, { 33, 0, 222 }, { 34, 0, 221 }, { 35, 0, 220 },
	{ 36, 0, 219 }, { 37, 0, 218 }, { 38, 0, 217 }, { 39, 0, 216 }, { 40, 0, 215 }, { 41, 0, 214 },
	{ 42, 0, 213 }, { 43, 0, 212 }, { 44, 0, 211 }, { 45, 0, 210 }, { 46, 0, 209 }, { 47, 0, 208 },
	{ 48, 0, 207 }, { 49, 0, 206 }, { 50, 0, 205 }, { 51, 0, 204 }, { 52, 0, 203 }, { 53, 0, 202 },
	{ 54, 0, 201 }, { 55, 0, 200 }, { 56, 0, 199 }, { 57, 0, 198 }, { 58, 0, 197 }, { 59, 0, 196 },
	{ 60, 0, 195 }, { 61, 0, 194 }, { 62, 0, 193 }, { 63, 0, 192 }, { 64, 0, 191 }, { 65, 0, 190 },
	{ 66, 0, 189 }, { 67, 0, 188 }, { 68, 0, 187 }, { 69, 0, 186 }, { 70, 0, 185 }, { 71, 0, 184 },
	{ 72, 0, 183 }, { 73, 0, 182 }, { 74, 0, 181 }, { 75, 0, 180 }, { 76, 0, 179 }, { 77, 0, 178 },
	{ 78, 0, 177 }, { 79, 0, 176 }, { 80, 0, 175 }, { 81, 0, 174 }, { 82, 0, 173 }, { 83, 0, 172 },
	{ 84, 0, 171 }, { 85, 0, 170 }, { 86, 0, 169 }, { 87, 0, 168 }, { 88, 0, 167 }, { 89, 0, 166 },
	{ 90, 0, 165 }, { 91, 0, 164 }, { 92, 0, 163 }, { 93, 0, 162 }, { 94, 0, 161 }, { 95, 0, 160 },
	{ 96, 0, 159 }, { 97, 0, 158 }, { 98, 0, 157 }, { 99, 0, 156 }, { 100, 0, 155 }, { 101, 0, 154 },
	{ 102, 0, 153 }, { 103, 0, 152 }, { 104, 0, 151 }, { 105, 0, 150 }, { 106, 0, 149 }, { 107, 0, 148 },
	{ 108, 0, 147 }, { 109, 0, 146 }, { 110, 0, 145 }, { 111, 0, 144 }, { 112, 0, 143 }, { 113, 0, 142 },
	{ 114, 0, 141 }, { 115, 0, 140 }, { 116, 0, 139 }, { 117, 0, 138 }, { 118, 0, 137 }, { 119, 0, 136 },
	{ 120, 0, 135 }, { 121, 0, 134 }, { 122, 0, 133 }, { 123, 0, 132 }, { 124, 0, 131 }, { 125, 0, 130 },
	{ 126, 0, 129 }, { 127, 0, 128 }, { 128, 0, 127 }, { 129, 0, 126 }, { 130, 0, 125 }, { 131, 0, 124 },
	{ 132, 0, 123 }, { 133, 0, 122 }, { 134, 0, 121 }, { 135, 0, 120 }, { 136, 0, 119 }, { 137, 0, 118 },
	{ 138, 0, 117 }, { 139, 0, 116 }, { 140, 0, 115 }, { 141, 0, 114 }, { 142, 0, 113 }, { 143, 0, 112 },
	{ 144, 0, 111 }, { 145, 0, 110 }, { 146, 0, 109 }, { 147, 0, 108 }, { 148, 0, 107 }, { 149, 0, 106 },
	{ 150, 0, 105 }, { 151, 0, 104 }, { 152, 0, 103 }, { 153, 0, 102 }, { 154, 0, 101 }, { 155, 0, 100 },
	{ 156, 0, 99 }, { 157, 0, 98 }, { 158, 0, 97 }, { 159, 0, 96 }, { 160, 0, 95 }, { 161, 0, 94 },
	{ 162, 0, 93 }, { 163, 0, 92 }, { 164, 0, 91 }, { 165, 0, 90 }, { 166, 0, 89 }, { 167, 0, 88 },
	{ 168, 0, 87 }, { 169, 0, 86 }, { 170, 0, 85 }, { 171, 0, 84 }, { 172, 0, 83 }, { 173, 0, 82 },
	{ 174, 0, 81 }, { 175, 0, 80 }, { 176, 0, 79 }, { 177, 0, 78 }, { 178, 0, 77 }, { 179, 0, 76 },
	{ 180, 0, 75 }, { 181, 0, 74 }, { 182, 0, 73 }, { 183, 0, 72 }, { 184, 0, 71 }, { 185, 0, 70 },
	{ 186, 0, 69 }, { 187, 0, 68 }, { 188, 0, 67 }, { 189, 0, 66 }, { 190, 0, 65 }, { 191, 0, 64 },
	{ 192, 0, 63 }, { 193, 0, 62 }, { 194, 0, 61 }, { 195, 0, 60 }, { 196, 0, 59 }, { 197, 0, 58 },
	{ 198, 0, 57 }, { 199, 0, 56 }, { 200, 0, 55 }, { 201, 0, 54 }, { 202, 0, 53 }, { 203, 0, 52 },
	{ 204, 0, 51 }, { 205, 0, 50 }, { 206, 0, 49 }, { 207, 0, 48 }, { 208, 0, 47 }, { 209, 0, 46 },
	{ 210, 0, 45 }, { 211, 0, 44 }, { 212, 0, 43 }, { 213, 0, 42 }, { 214, 0, 41 }, { 215, 0, 40 },
	{ 216, 0, 39 }, { 217, 0, 38 }, { 218, 0, 37 }, { 219, 0, 36 }, { 220, 0, 35 }, { 221, 0, 34 },
	{ 222, 0, 33 }, { 223, 0, 32 }, { 224, 0, 31 }, { 225, 0, 30 }, { 226, 0, 29 }, { 227, 0, 28 },
	{ 228, 0, 27 }, { 229, 0, 26 }, { 230, 0, 25 }, { 231, 0, 24 }, { 232, 0, 23 }, { 233, 0, 22 },
	{ 234, 0, 21 }, { 235, 0, 20 }, { 236, 0, 19 }, { 237, 0, 18 }, { 238, 0, 17 }, { 239, 0, 16 },
	{ 240, 0, 15 }, { 241, 0, 14 }, { 242, 0, 13 }, { 243, 0, 12 }, { 244, 0, 11 }, { 245, 0, 10 },
	{ 246, 0, 9 }, { 247, 0, 8 }, { 248, 0, 7 }, { 249, 0, 6 }, { 250, 0, 5 }, { 251, 0, 4 },
	{ 252, 0, 3 }, { 253, 0, 2 }, { 254, 0, 1 }, { 255, 0, 0 }, };

double calculateVariance(const int data[], int n) {
	if (n <= 1) {
		//方差需要至少两个数据点
		return 0.0;
	}

	// 计算平均值
		double mean = 0.0;
	for (int i = 0; i < n; i++) {
		mean += data[i];
	}
	mean /= n;

	// 计算差异的平方和
	double sumSquaredDiff = 0.0;
	for (int i = 0; i < n; i++) {
		double diff = data[i] - mean;
		sumSquaredDiff += diff * diff;
	}

	// 计算方差
	double variance = sumSquaredDiff / (n - 1);

	return variance;
}

void pbox_light_effect_soundreactive_analysis(energy_data_t energy_data)
{
	double variance = 0.0;
	ctrl->energy_total = 0;
	double standarddeviation = 0.0;


	for (int i = 0; i < energy_data.size; i++) {
		ctrl->energy_total += abs(energy_data.energykeep[i].energy);
	}

	ctrl->energy_total_data_record[ctrl->energy_data_index] = ctrl->energy_total;
	ctrl->energy_data_index++;
	if (ctrl->energy_data_index >= ARRAY_SIZE(ctrl->energy_total_data_record))
		ctrl->energy_data_index = 0;

	ctrl->energy_total_average = 0;
	for (int i = 0; i < ARRAY_SIZE(ctrl->energy_total_data_record); i++) {
		ctrl->energy_total_average += ctrl->energy_total_data_record[i];
	}
	ctrl->energy_total_average = ctrl->energy_total_average / ARRAY_SIZE(ctrl->energy_total_data_record);

	variance = calculateVariance(ctrl->energy_total_data_record, ARRAY_SIZE(ctrl->energy_total_data_record));

	//ALOGI("==========variance:%lf standarddeviation:%lf energy_total:%d energy_total_average:%d============\n",variance,sqrt(variance),ctrl->energy_total, ctrl->energy_total_average);


	if ((ctrl->energy_total  < ctrl->energy_total_average))
		leffect->led_effect_type =  9;
	else
		leffect->led_effect_type =  10;
}

void get_rgb_form_color_wheel(int step, int *r, int *g, int *b)
{

	*r = color_wheel[ctrl->colors_wheel_index][0];
	*g = color_wheel[ctrl->colors_wheel_index][1];
	*b = color_wheel[ctrl->colors_wheel_index][2];

	ctrl->colors_wheel_index = ctrl->colors_wheel_index + step;
	if (ctrl->colors_wheel_index >= ARRAY_SIZE(color_wheel))
		ctrl->colors_wheel_index = 0;
}

int mode_switching = 0;

int led_effect_switching(int step)
{
	int i,colorIndex;
	int r, g, b;

	while (step --) {
		for (i = 0; i < ctrl->unit_num; i++) {
			colorIndex = (ctrl->colors_wheel_index + i * (ARRAY_SIZE(color_wheel) / ctrl->unit_num)) % ARRAY_SIZE(color_wheel);
			userspace_set_rgb_color(ctrl, i, color_wheel[colorIndex][0], color_wheel[colorIndex][1], color_wheel[colorIndex][2]);
		}
		ctrl->colors_wheel_index += (ARRAY_SIZE(color_wheel) / ctrl->unit_num);
		ctrl->colors_wheel_index %= ARRAY_SIZE(color_wheel);
	}
}

int led_effect_egre_ebb(int action)
{
	int r, g, b;
	int end = 0;

	cal_data->steps_time = 10;
	
	if(ctrl->light_effect_drew)
		return 0;

	if(mode_switching)
		led_effect_switching(500);

	mode_switching = 0;


	get_rgb_form_color_wheel(5, &r, &g, &b);

	if (action == 0) {
		end = ctrl->unit_num/2;
		for (int j = 0; j < end; j++) {
			userspace_set_rgb_color(ctrl, j, r*(end-j)/end, g*(end-j)/end, b*(end-j)/end);
			userspace_set_rgb_color(ctrl, ctrl->unit_num -1 - j, r*(end-j)/end, g*(end-j)/end, b*(end-j)/end);
			msleep(20);
		}
	} else if (action == 1) {
		end = rand()%2;
		for (int j = ctrl->unit_num/2; j > end; j--) {
			userspace_set_rgb_color(ctrl, j, 0, 0, 0);
			userspace_set_rgb_color(ctrl, ctrl->unit_num -1 - j, 0, 0, 0);
			msleep(10);
		}
		userspace_set_rgb_color(ctrl, end - 1, r/2, g/2, b/2);
		userspace_set_rgb_color(ctrl, ctrl->unit_num - end, r/2, g/2, b/2);
	} else if (action == 2) {
		end = ctrl->unit_num/2;
		userspace_set_rgb_color(ctrl, 0, r, g, b);
		for (int j = 1; j < end; j++) {
			userspace_set_rgb_color(ctrl, j, r*(end-j)/end, g*(end-j)/end, b*(end-j)/end);
			userspace_set_rgb_color(ctrl, ctrl->unit_num - j, r*(end-j)/end, g*(end-j)/end, b*(end-j)/end);
			msleep(20);
		}
		userspace_set_rgb_color(ctrl, end, r/end, g/end, b/end);
	} else if (action == 3) {
		userspace_set_rgb_color(ctrl, ctrl->unit_num/2, 0, 0, 0);
		end = rand()%2 + 1;
		for (int j = 0; j < (ctrl->unit_num/2 - end); j++) {
			userspace_set_rgb_color(ctrl, ctrl->unit_num/2 - j, 0, 0, 0);
			userspace_set_rgb_color(ctrl, ctrl->unit_num/2 + j, 0, 0, 0);
			msleep(10);
		}
		userspace_set_rgb_color(ctrl, ctrl->unit_num - end, r/2, g/2, b/2);
		userspace_set_rgb_color(ctrl, end, r/2, g/2, b/2);
		userspace_set_rgb_color(ctrl, 0, r, g, b);
	}
	ctrl->light_effect_drew = 0;
}

int led_effect_volume_analysis(int type, int volume)
{
	if (type == 1) {
		foreground_leffect->led_effect_type =  1;
		foreground_leffect->start = 1;
		foreground_leffect->fore_color = 0x00ff00;
		if (volume) {
			foreground_leffect->num = (volume + ctrl->unit_num - 1) * ctrl->unit_num / 100;
		} else {
			foreground_leffect->num = 0;
		}
		foreground_leffect->period = 500;
		//ALOGD("===%s:volume:%d num:%d color%d====\n", __func__, volume, foreground_leffect.num, foreground_leffect.fore_color);
	} else if (type == 2){
		foreground_leffect->led_effect_type =  2;
		foreground_leffect->start = 1;
		if (volume) {
			foreground_leffect->num = (volume + ctrl->unit_num - 1) * ctrl->unit_num / 100;
			foreground_leffect->fore_color = 0x00ff00;
			//ALOGD("===%s:volume:%d num:%d color%d====\n", __func__, volume, foreground_leffect.num, foreground_leffect.fore_color);
		}else {
			foreground_leffect->fore_color = 0xff0000;
			foreground_leffect->num = ctrl->unit_num;
			//ALOGD("===%s:volume:%d num:%d color%d====\n", __func__, volume, foreground_leffect.num, foreground_leffect.fore_color);
		}

		foreground_leffect->period = 1000;
		//ALOGD("===%s:volume:%d num:%d====\n", __func__, volume, foreground_leffect.num);
	}
		foreground_leffect_job = 1;
}

void led_effect_init_bright(struct led_effect* effect,int init_led)
{
	int i;

	memset(cal_data->force_bright,0,sizeof(cal_data->force_bright));
	memset(cal_data->back_bright,0,sizeof(cal_data->back_bright));

	for (i = effect->start*3; i < effect->num*3; i++) {
		if(i%3 == 0)
			cal_data->force_bright[i] = (effect->fore_color & 0x00FF00) >> 8;
		if(i%3 == 1)
			cal_data->force_bright[i] = (effect->fore_color & 0xFF0000) >> 16;
		if(i%3 == 2)
			cal_data->force_bright[i] = (effect->fore_color & 0x0000FF);

		cal_data->bright[i] = cal_data->force_bright[i];

		if(i%3 == 0)
			cal_data->back_bright[i] = (effect->back_color & 0x00FF00) >> 8;
		if(i%3 == 1)
			cal_data->back_bright[i] = (effect->back_color & 0xFF0000) >> 16;
		if(i%3 == 2)
			cal_data->back_bright[i] = (effect->back_color & 0x0000FF);

		if(init_led == FG_COLOR) {
			cal_data->step_bright[i] = abs(cal_data->back_bright[i] - cal_data->force_bright[i]) / effect->actions_per_period;
			// pk_debug("step_brightness[%d]=%d",i,cal_data->step_bright[i]);
			// pk_debug("start_bright[%d]=%d",i,cal_data->force_bright[i]);
			// pk_debug("end_brightness[%d]=%d",i,cal_data->back_bright[i]);
			userspace_set_led_brightness(ctrl, i, cal_data->force_bright[i]);
		}
		if(init_led == BG_COLOR) {
			// cal_data->step_bright[i] = abs(cal_data->back_bright[i] - cal_data->force_bright[i]) / effect->actions_per_period;
			userspace_set_led_brightness(ctrl, i, cal_data->back_bright[i]);
		}
	}
}

void led_forcebright_to_back(int i,struct led_effect* effect)
{
	int x;
	if(cal_data->force_bright[i] < cal_data->back_bright[i]) {
		cal_data->bright[i] += cal_data->step_bright[i];

		userspace_set_led_brightness(ctrl, i,cal_data->bright[i]);
		if(cal_data->bright[i] >= cal_data->back_bright[i]) {
			for (x = effect->start*3; x < effect->num*3; x++) {
				cal_data->bright[x] = cal_data->back_bright[x];
				userspace_set_led_brightness(ctrl, x, cal_data->bright[x]);
			}
			cal_data->data_valid = 2;
			// pk_debug("led[%d]=%d",i,cal_data->bright[i]);
		}
		// pk_debug("led[%d]=%d",i,cal_data->bright[i]);
		userspace_set_led_brightness(ctrl, i, cal_data->bright[i]);
	}

	if(cal_data->force_bright[i] > cal_data->back_bright[i]) {
		cal_data->bright[i] -= cal_data->step_bright[i];

		userspace_set_led_brightness(ctrl, i, cal_data->bright[i]);
		if(cal_data->bright[i] <= cal_data->back_bright[i]) {
			for (x = effect->start*3; x < effect->num*3; x++) {
				cal_data->bright[x] = cal_data->back_bright[x];
				userspace_set_led_brightness(ctrl, x, cal_data->bright[x]);
			}
			cal_data->data_valid = 2;
			// pk_debug("led[%d]=%d",i,cal_data->bright[i]);
		}
		// pk_debug("led[%d]=%d\n",i,cal_data->bright[i]);
	}
}

void led_backbright_to_force(int i,struct led_effect* effect)
{
	int x;
	if(cal_data->force_bright[i] < cal_data->back_bright[i]) {
		cal_data->bright[i] -= cal_data->step_bright[i];
		// pk_debug("led[%d]=%d",i,cal_data->bright[i]);
		userspace_set_led_brightness(ctrl, i, cal_data->bright[i]);
		if(cal_data->bright[i] <= cal_data->force_bright[i]) {
			for (x = effect->start*3; x < effect->num*3; x++) {
				cal_data->bright[x] = cal_data->force_bright[x];
				userspace_set_led_brightness(ctrl, x, cal_data->bright[x]);
			}
			cal_data->data_valid = 1;
			// pk_debug("led[%d]=%d",i,cal_data->bright[i]);
		}

	}

	if(cal_data->force_bright[i] > cal_data->back_bright[i]) {
		cal_data->bright[i] += cal_data->step_bright[i];

		userspace_set_led_brightness(ctrl, i, cal_data->bright[i]);
		// pk_debug("led[%d]=%d",i,cal_data->bright[i]);
		if(cal_data->bright[i] >= cal_data->force_bright[i]) {
			for (x = effect->start*3; x < effect->num*3; x++) {
				cal_data->bright[x] = cal_data->force_bright[x];
				userspace_set_led_brightness(ctrl, x, cal_data->bright[x]);
			}
			cal_data->data_valid = 1;
			// pk_debug("led[%d]=%d",i,cal_data->bright[i]);
		}

	}
}

static void led_effect_breath(struct led_effect* effect)
{
	int i = 0;
	// pk_debug("cal_data->data_valid=%d",cal_data->data_valid);

	if(cal_data->data_valid == 0) {
		led_effect_init_bright(effect,FG_COLOR);
		cal_data->data_valid = 1;
	}

	for (i = effect->start*3; i < effect->num*3; i++) {
		// pk_debug("cal_data->data_valid=%d",cal_data->data_valid);
		if(cal_data->data_valid == 1) {
			// pk_debug("cal_data->data_valid=%d",cal_data->data_valid);
			led_forcebright_to_back(i,effect);
		}
		if(cal_data->data_valid == 2) {
			// pk_debug("cal_data->data_valid=%d",cal_data->data_valid);
			led_backbright_to_force(i,effect);
		}
	}
}

static void led_effect_fade(struct led_effect* effect)
{
	int i;
	 //ALOGD("cal_data->data_valid=%d\n",cal_data->data_valid);
	if(cal_data->data_valid == 0) {
		led_effect_init_bright(effect,FG_COLOR);
		cal_data->data_valid = 1;
	}

	for (i = effect->start*3; i < effect->num*3; i++) {
		//ALOGD("red[%d]=%d\n",i,cal_data->bright[i]);
		if(cal_data->data_valid == 1 ) {
			led_forcebright_to_back(i,effect);
		}

		if(cal_data->data_valid == 2) {
			memset(cal_data->force_bright,0,sizeof(cal_data->force_bright));
			memset(cal_data->back_bright,0,sizeof(cal_data->back_bright));
		}
	}
}

static int led_effect_blink(struct led_effect* effect)
{
	int i;
	// pk_debug("cal_data->data_valid=%d\n",cal_data->data_valid);

	switch (cal_data->data_valid)
	{
		case 0:
			led_effect_init_bright(effect,FG_COLOR);
			cal_data->data_valid = 1;
			break;
		case 1:
			for (i = effect->start*3; i < effect->num*3; i++) {
				// pk_debug("red[%d]=%d",i,cal_data->bright[i]);
				cal_data->bright[i] = 0;
				userspace_set_led_brightness(ctrl, i, cal_data->bright[i]);
			}
			cal_data->data_valid = 0;
			break;
		default:
			ALOGW("led data_valid not used");
			break;
	}

	return 0;
}

//互换
static void led_effect_exchange(struct led_effect* effect)
{
	// pk_debug("cal_data->data_valid=%d\n",cal_data->data_valid);

	switch (cal_data->data_valid)
	{
		case 0:
			led_effect_init_bright(effect,FG_COLOR);
			cal_data->data_valid = 1;
			break;
		case 1:
			led_effect_init_bright(effect,BG_COLOR);
			cal_data->data_valid = 0;
			break;
		default:
			ALOGW("led data_valid not used");
			break;
	}
}

//跑马灯
static int led_effect_marquee(struct led_effect* effect)
{
	int led_bit;
	int x;
	if(cal_data->data_valid == 0){
		led_effect_init_bright(effect,BG_COLOR);
		cal_data->data_valid = 1;
		led_bit=0;
	}

	if(cal_data->data_valid == 1){
		for(x = effect->start*3; x < (led_bit+3*effect->scroll_num-3); x++) {
			userspace_set_led_brightness(ctrl, x, cal_data->bright[x]);
		}

		if((led_bit+3*effect->scroll_num) > effect->num*3) {
			led_bit=0;
		}
		for(x=led_bit;x < (led_bit+3*effect->scroll_num); x++) {
			userspace_set_led_brightness(ctrl, x, cal_data->bright[x]);
		}
		led_bit = led_bit+3;

		// if((led_bit+3*effect->scroll_num) >= (effect->num*3+effect->scroll_num+3)) {
		//     // for (x = effect->start*3; x < effect->num*3; x++){
		//     //     set_led_data(x, (effect->num*3-1),cal_data->back_bright[x]);
		//     // }
		//     pk_debug("-----------------led[%d]=%d",led_bit,cal_data->bright[led_bit]);
		//     led_bit=0;
		// }

		// if(led_bit >= effect->num*3+3) {
		//     led_bit=0;
		// }
	}

	return 0;
}

static int led_effect_light_add(struct led_effect* effect)
{
	int i;
	int x;
	if(cal_data->data_valid == 0){
		led_effect_init_bright(effect,BG_COLOR);
		cal_data->data_valid = 1;
		i=0;
	}

	if(cal_data->data_valid == 2){
		i=i-3;
		if(i < 0) {
			cal_data->data_valid = 1;
			i=0;
		}
		for(x = i;x < (i+3); x++){
			userspace_set_led_brightness(ctrl, x, cal_data->bright[x]);
		}
	}

	if(cal_data->data_valid == 1){
		for(x=i;x < (i+3); x++){
			userspace_set_led_brightness(ctrl, x, cal_data->bright[x]);
		}
		i=i+3;
		if(i >= (effect->num*3)) {
			cal_data->data_valid = 2;
			i = effect->num*3;
		}
	}

	return 0;
}

//一个灯以RGB方式循环
static int led_effect_scroll_one(struct led_effect* effect)
{
	int i;
	static int x;
	int brightness[3];
	int bri;

	// pk_debug("cal_data->data_valid=%d\n",cal_data->data_valid);
	if(cal_data->data_valid == 0) {
		x = 0;
		cal_data->data_valid = 1;
	}
	memset(brightness,0,sizeof(brightness));
	if(cal_data->data_valid == 1) {
		brightness[x] = (effect->fore_color & (0x0000FF<<((2-x)*8)));
		for (i = effect->start; i < effect->num*3; i++) {
			bri = brightness[i] >> ((2-i)*8);
			userspace_set_led_brightness(ctrl, i, bri);
		}
		x++;
		if(x > (effect->num*3-1)) {
			x=0;
		}
	}
	return 0;
}

//常亮
static void led_effect_open(struct led_effect* effect)
{
	led_effect_init_bright(effect,FG_COLOR);
}

static void led_effect_close(struct led_effect* effect)
{
	int i;
	for (i = effect->start*3; i < effect->num*3; i++) {
		cal_data->bright[i] = 0;
		userspace_set_led_brightness(ctrl, i, cal_data->bright[i]);
	}

	// pk_debug("timer_status=%d",timer_status);
	if(ctrl->light_effect_drew) {
		ctrl->light_effect_drew = 0;
	}
}
int led_effect_handle(struct led_effect *effect)
{
	int ret = 0;

	int total_num;
	total_num = get_led_total_num(ctrl);

	if(effect->start*3 > effect->num*3 || effect->start*3 > (total_num -1)) {
		ALOGE("ERROR: %s(%d): led start %d > end %d\n", __func__, __LINE__, effect->start*3, effect->num*3);
		return -EINVAL;
	}

	if((effect->num*3-1) > (total_num-1)) {
		ALOGE("ERROR: %s(%d): led end %d > led pins %d\n", __func__, __LINE__, effect->num*3, total_num);
		return -EINVAL;
	}

	if((effect->scroll_num*3 -1) > (total_num-1)) {
		ALOGE("ERROR: %s(%d): led scroll_num %d > led pins %d\n", __func__, __LINE__, effect->scroll_num*3, total_num);
		return -EINVAL;
	}


	// pk_debug("period=%d,back_color=%x,fore_color=%x,start=%d,num=%d,scroll_num=%d,actions_per_period=%d.led_effect_type=%d",
	// 	leffect->period,leffect->back_color,leffect->fore_color,leffect->start,leffect->num,leffect->scroll_num,
	// 	leffect->actions_per_period,leffect->led_effect_type);

	cal_data->data_valid = 0;

	led_effect_close(leffect);
	if(leffect->led_effect_type == 0 || leffect->led_effect_type == 1) {
		switch(leffect->led_effect_type)
		{
			case 0: led_effect_close(leffect);
				break;
			case 1: led_effect_open(leffect);
				break;
			default:break;
		}
	} else {
		cal_data->steps_time = (leffect->period / 2)/ leffect->actions_per_period ;
		ALOGD("steps_time = %d",cal_data->steps_time);
	}

	return ret;
}

int led_effect_volume(struct led_effect* effect)
{
	int i;
	for (i = 0; i < effect->num; i++) {
		userspace_set_rgb_color(ctrl, i,
				(foreground_leffect->fore_color&0xff0000) >>16,
				(foreground_leffect->fore_color&0x00ff00) >>8,
				(foreground_leffect->fore_color&0x0000ff));
	}
	for (i = effect->num; i < ctrl->unit_num; i++) {
		userspace_set_rgb_color(ctrl, i, 0, 0, 0);
	}
	msleep(effect->period);
	for (i = 0; i < effect->num; i++) {
		userspace_set_rgb_color(ctrl, i, 0, 0, 0);
	}
	foreground_leffect_job = 0;
}

void *pbox_light_effect_drew(void *para)
{
	while (true) {
		//ALOGD("%s:%d leffect->led_effect_type:%d cal_data->steps_time:%d ctrl->soundreactive_mute %d\n", __func__, __LINE__, leffect->led_effect_type, cal_data->steps_time, ctrl->soundreactive_mute);
		if (foreground_leffect_job) {
			if(foreground_leffect->led_effect_type == 1 || foreground_leffect->led_effect_type == 2)
				led_effect_volume(foreground_leffect);
			msleep(20);

		}
		else {
			if (leffect->led_effect_type == 2 && (ctrl->soundreactive_mute))
				led_effect_breath(leffect);
			else if(leffect->led_effect_type == 3 && (ctrl->soundreactive_mute))
				led_effect_fade(leffect);
			else if(leffect->led_effect_type == 4 && (ctrl->soundreactive_mute))
				led_effect_blink(leffect);
			else if(leffect->led_effect_type == 5 && (ctrl->soundreactive_mute))
				led_effect_exchange(leffect);
			else if(leffect->led_effect_type == 6 && (ctrl->soundreactive_mute))
				led_effect_marquee(leffect);
			else if(leffect->led_effect_type == 7 && (ctrl->soundreactive_mute))
				led_effect_light_add(leffect);
			else if(leffect->led_effect_type == 8 && (ctrl->soundreactive_mute))
				led_effect_scroll_one(leffect);
			else if(leffect->led_effect_type == 9 && (!ctrl->soundreactive_mute))
				led_effect_egre_ebb(2);
			else if(leffect->led_effect_type == 10 && (!ctrl->soundreactive_mute))
				led_effect_egre_ebb(3);

			msleep(cal_data->steps_time);
			continue;
		}
	}
}

void soundreactive_mute_set(bool mute){
	if (mute) {
		ctrl->soundreactive_mute = 1;
		mode_switching = 1;
	}
	else {
		ctrl->soundreactive_mute = 0;
	}
}

void pbox_light_effect_soundreactive(energy_data_t energy_data)
{
	if (ctrl->soundreactive_mute) {
		//ALOGD("%s:%d return\n", __func__, __LINE__);
		return;
	}
	//userspace_set_led_effect(RK_ECHO_LED_OFF);
	pbox_light_effect_soundreactive_analysis(energy_data);
}

int userspace_set_led_effect(struct light_effect_ctrl * ctrl, char *led_effect_name)
{
	if(get_led_effect_data(ctrl, leffect, led_effect_name) < 0)
		return -1;

	led_effect_handle(leffect);
}

#if ENABLE_RK_LED_EFFECT
static void *pbox_light_effect_server(void *arg)
{
	int light_effect_fds[1] = {0};
	int maxfd;
	char buff[sizeof(pbox_light_effect_msg_t)] = {0};
	pbox_light_effect_msg_t *msg;


	pthread_setname_np(pthread_self(), "party_light_effect");

	int sock_fd = get_server_socketpair_fd(PBOX_SOCKPAIR_LED);

	if(sock_fd < 0)
		return (void *)-1;

	light_effect_fds[0] = sock_fd;

	fd_set read_fds;
	FD_ZERO(&read_fds);
	FD_SET(sock_fd, &read_fds);

	while(true) {

		FD_ZERO(&read_fds);
		FD_SET(sock_fd, &read_fds);

		int result = select(sock_fd + 1, &read_fds, NULL, NULL, NULL);
		if (result < 0) {
			if (errno != EINTR) {
				perror("select failed");
				break;
			}
			continue; // Interrupted by signal, restart select
		} else if (result == 0) {
			ALOGW("select timeout or no data\n");
			continue;
		}
		int ret = recv(sock_fd, buff, sizeof(buff), 0);
		if (ret <= 0)
			continue;

		pbox_light_effect_msg_t *msg = (pbox_light_effect_msg_t *)buff;
		//ALOGD("%s recv: type: %d, id: %d\n", __func__, msg->type, msg->msgId);

		if(msg->type == PBOX_EVT)
			continue;

		switch (msg->msgId) {
			case PBOX_LIGHT_EFFECT_SOUNDREACTIVE_EVT:
				pbox_light_effect_soundreactive(msg->energy_data);
				break;
			case RK_ECHO_SYSTEM_BOOTING_EVT:
				userspace_set_led_effect(ctrl, RK_ECHO_SYSTEM_BOOTING);
				break;
			case RK_ECHO_SYSTEM_BOOTC_EVT:
				userspace_set_led_effect(ctrl, RK_ECHO_SYSTEM_BOOTC);
				break;
			case RK_ECHO_NET_CONNECT_RECOVERY_EVT:
				userspace_set_led_effect(ctrl, RK_ECHO_NET_CONNECT_RECOVERY);
				break;
			case RK_ECHO_NET_CONNECT_WAITTING_EVT:
				userspace_set_led_effect(ctrl, RK_ECHO_NET_CONNECT_WAITTING);
				break;
			case RK_ECHO_NET_CONNECTING_EVT:
				userspace_set_led_effect(ctrl, RK_ECHO_NET_CONNECTING);
				break;
			case RK_ECHO_NET_CONNECT_FAIL_EVT:
				userspace_set_led_effect(ctrl, RK_ECHO_NET_CONNECT_FAIL);
				break;
			case RK_ECHO_NET_CONNECT_SUCCESS_EVT:
				userspace_set_led_effect(ctrl, RK_ECHO_NET_CONNECT_SUCCESS);
				break;
			case RK_ECHO_WAKEUP_WAITTING_EVT:
				userspace_set_led_effect(ctrl, RK_ECHO_WAKEUP_WAITTING);
				break;
			case RK_ECHO_TTS_THINKING_EVT:
				userspace_set_led_effect(ctrl, RK_ECHO_TTS_THINKING);
				break;
			case RK_ECHO_TTS_PLAYING_EVT:
				userspace_set_led_effect(ctrl, RK_ECHO_TTS_PLAYING);
				break;
			case RK_ECHO_BT_PAIR_WAITTING_EVT:
				userspace_set_led_effect(ctrl, RK_ECHO_BT_PAIR_WAITTING);
				break;
			case RK_ECHO_BT_PAIRING_EVT:
				userspace_set_led_effect(ctrl, RK_ECHO_BT_PAIRING);
				break;
			case RK_ECHO_BT_PAIR_FAIL_EVT:
				userspace_set_led_effect(ctrl, RK_ECHO_BT_PAIR_FAIL);
				break;
			case RK_ECHO_BT_PAIR_SUCCESS_EVT:
				userspace_set_led_effect(ctrl, RK_ECHO_BT_PAIR_SUCCESS);
				break;
			case RK_ECHO_VOLUME_LED_EVT:
				led_effect_volume_analysis(1,msg->volume);
				break;
			case RK_ECHO_MIC_MUTE_EVT:
				led_effect_volume_analysis(2,msg->mic_volume);
				break;
			case RK_ECHO_MIC_UNMUTE_EVT:
				userspace_set_led_effect(ctrl, RK_ECHO_MIC_UNMUTE);
				break;
			case RK_ECHO_ALARM_EVT:
				userspace_set_led_effect(ctrl, RK_ECHO_ALARM);
				break;
			case RK_ECHO_UPGRADING_EVT:
				userspace_set_led_effect(ctrl, RK_ECHO_UPGRADING);
				break;
			case RK_ECHO_UPGRADE_END_EVT:
				userspace_set_led_effect(ctrl, RK_ECHO_UPGRADE_END);
				break;
			case RK_ECHO_LED_OFF_EVT:
				userspace_set_led_effect(ctrl, RK_ECHO_LED_OFF);
				break;
			case RK_ECHO_CHARGER_EVT:
				userspace_set_led_effect(ctrl, RK_ECHO_CHARGER);
				break;
			case RK_ECHO_LOW_BATTERY_EVT:
				userspace_set_led_effect(ctrl, RK_ECHO_LOW_BATTERY);
				break;
			case RK_ECHO_PHONE_EVT:
				userspace_set_led_effect(ctrl, RK_ECHO_PHONE);
				break;
			case RK_ECHO_TIME_EVT:
				userspace_set_led_effect(ctrl, RK_ECHO_TIME);
				break;
			case RK_ECHO_TEST_EVT:
				userspace_set_led_effect(ctrl, RK_ECHO_TEST);
				break;
			case RK_ECHO_PLAY_EVT:
				soundreactive_mute_set(false);
				userspace_set_led_effect(ctrl, RK_ECHO_PLAY);
				break;
			case RK_ECHO_PAUSE_EVT:
				soundreactive_mute_set(true);
				userspace_set_led_effect(ctrl, RK_ECHO_PAUSE);
				break;
			case RK_ECHO_PLAY_NEXT_EVT:
				soundreactive_mute_set(true);
				userspace_set_led_effect(ctrl, RK_ECHO_PLAY_NEXT);
				break;
			case RK_ECHO_PLAY_PREV_EVT:
				soundreactive_mute_set(true);
				userspace_set_led_effect(ctrl, RK_ECHO_PLAY_PREV);
				break;
			default: {
				 } break;
		}
	}
}
#endif

int effect_calcule_data_init(void)
{
	int total_num;
	cal_data = os_malloc(sizeof(struct effect_calcule_data));
	if (!cal_data) {
		ALOGE("%s:effect_calcule_data alloc failed\n", __func__);
		goto exit5;
	}
	memset(cal_data, 0x00, sizeof(struct effect_calcule_data));

	total_num = get_led_total_num(ctrl);

	cal_data->force_bright = os_malloc(sizeof(int) * total_num);
	if (!cal_data->force_bright) {
		ALOGE("%s:cal_data->force_bright alloc failed\n", __func__);
		goto exit4;
	}
	memset(cal_data->force_bright, 0x00, sizeof(int) * total_num);

	cal_data->back_bright = os_malloc(sizeof(int) * total_num);
	if (!cal_data->back_bright) {
		ALOGE("%s:cal_data->back_bright alloc failed\n", __func__);
		goto exit3;
	}
	memset(cal_data->back_bright, 0x00, sizeof(int) * total_num);

	cal_data->bright = os_malloc(sizeof(int) * total_num);
	if (!cal_data->bright) {
		ALOGE("%s:cal_data->bright alloc failed\n", __func__);
		goto exit2;
	}
	memset(cal_data->bright, 0x00, sizeof(int) * total_num);
	
	cal_data->step_bright = os_malloc(sizeof(int) * total_num);
	if (!cal_data->step_bright) {
		ALOGE("%s:cal_data->step_bright alloc failed\n", __func__);
		goto exit1;
	}
	memset(cal_data->step_bright, 0x00, sizeof(int) * total_num);

	return 0;
exit1:
	os_free(cal_data->bright);
exit2:
	os_free(cal_data->back_bright);
exit3:
	os_free(cal_data->force_bright);
exit4:
	os_free(cal_data);
exit5:
	return -1;
}

int effect_calcule_data_deinit(void)
{
	os_free(cal_data->step_bright);
	os_free(cal_data->bright);
	os_free(cal_data->back_bright);
	os_free(cal_data->force_bright);
	os_free(cal_data);
	return 0;
}

int base_light_config_deinit(void)
{
	os_free(ctrl->position_mapp);
	os_free(ctrl->unit_fd);
	os_free(ctrl);
}

int pbox_light_effect_init(void)
{
	int total_num;

	if (ctrl) {
		ALOGE("%s:already inited,direct reutrn\n", __func__);
		return 0;
	}

	ctrl = os_malloc(sizeof(struct light_effect_ctrl));
	if (!ctrl) {
		ALOGE("%s:light_effect_ctrl alloc failed\n", __func__);
		goto exit4;
	}
	memset(ctrl, 0x00, sizeof(struct light_effect_ctrl));

	if(base_light_config_init(ctrl, "led_base_config"))
		goto exit3;

	ctrl->soundreactive_mute = 1;

	leffect = os_malloc(sizeof(struct led_effect));
	if (!leffect) {
		ALOGE("%s:led_effect alloc failed\n", __func__);
		goto exit3;
	}
	memset(leffect, 0x00, sizeof(struct led_effect));

	foreground_leffect = os_malloc(sizeof(struct led_effect));
	if (!foreground_leffect) {
		ALOGE("%s:led_effect alloc failed\n", __func__);
		goto exit2;
	}
	memset(foreground_leffect, 0x00, sizeof(struct led_effect));

	if (effect_calcule_data_init())
		goto exit1;

	led_userspace_ctrl_init(ctrl);

	return 0;
exit1:
	os_free(foreground_leffect);
exit2:
	os_free(leffect);
exit3:
	os_free(ctrl);
exit4:
	return -1;
}

int pbox_light_effect_deinit(struct light_effect_ctrl * ctrl)
{
	if (!ctrl) {
		ALOGE("%s:already deinited,direct reutrn\n", __func__);
		return 0;
	}

	led_userspace_ctrl_deinit(ctrl);
	effect_calcule_data_deinit();
	os_free(foreground_leffect);
	os_free(leffect);
	base_light_config_deinit();

	ctrl = NULL;
}

pthread_t light_effect_task_id;
pthread_t light_effect_drew_id;

#if ENABLE_RK_LED_EFFECT
int pbox_create_lightEffectTask(void)
{
	int ret;

	ret = pbox_light_effect_init();
	if (ret < 0) {
		ALOGE("pbox light effect init failed\n");
		return ret;
	}

	ret = pthread_create(&light_effect_task_id, NULL, pbox_light_effect_server, NULL);
	if (ret < 0)
		ALOGE("light effect server start failed\n");

	ret = pthread_create(&light_effect_drew_id, NULL,pbox_light_effect_drew, NULL);
	if (ret < 0)
		ALOGE("light effect drew start failed\n");

	return ret;
}
#endif

int pbox_light_effect_send_cmd(pbox_light_effect_opcode_t command, void *data, int len)
{
	pbox_light_effect_msg_t msg = {0};

	msg.type = RK_LIGHT_EFFECT_CMD;
	msg.msgId = command;
	if (msg.msgId == PBOX_LIGHT_EFFECT_SOUNDREACTIVE_EVT) {
		if(data != NULL)
			memcpy(&msg.energy_data, data, sizeof(msg.energy_data));
	} else if (msg.msgId == RK_ECHO_VOLUME_LED_EVT) {
		if(data != NULL)
			memcpy(&msg.volume, data, sizeof(msg.volume));
	} else if (msg.msgId == RK_ECHO_MIC_MUTE_EVT) {
		if(data != NULL)
			memcpy(&msg.mic_volume, data, sizeof(msg.mic_volume));
	}

	#if ENABLE_RK_LED_EFFECT
	unix_socket_send_cmd(PBOX_CHILD_LED,&msg, sizeof(pbox_light_effect_msg_t));
	#endif
}
