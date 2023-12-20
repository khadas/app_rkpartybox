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

#include "pbox_light_effect.h"
#include "pbox_socket.h"
#include "pbox_common.h"
#include "pthread.h"
#include <errno.h>
#include <stdbool.h>
#include "pbox_ledctrl.h"

extern light_effect_ctrl_t light_effect_ctrl;
#define msleep(x) usleep(x * 1000)


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

void pbox_light_effect_soundreactive_1(energy_data_t energy_data)
{
	double variance = 0.0;
	light_effect_ctrl.energy_total = 0;
	double standarddeviation = 0.0;


	for (int i = 0; i < energy_data.size; i++) {
		light_effect_ctrl.energy_total += energy_data.energykeep[i].energy;
	}

	light_effect_ctrl.energy_total_data_record[light_effect_ctrl.energy_data_index] = light_effect_ctrl.energy_total;
	light_effect_ctrl.energy_data_index++;
	if (light_effect_ctrl.energy_data_index >= sizeof(light_effect_ctrl.energy_total_data_record)/sizeof(light_effect_ctrl.energy_total_data_record[0]))
		light_effect_ctrl.energy_data_index = 0;

	light_effect_ctrl.energy_total_average = 0;
	for (int i = 0; i < sizeof(light_effect_ctrl.energy_total_data_record)/sizeof(light_effect_ctrl.energy_total_data_record[0]); i++) {
		light_effect_ctrl.energy_total_average += light_effect_ctrl.energy_total_data_record[i];
	}
	light_effect_ctrl.energy_total_average = light_effect_ctrl.energy_total_average / (sizeof(light_effect_ctrl.energy_total_data_record)/sizeof(light_effect_ctrl.energy_total_data_record[0]));

	variance = calculateVariance(light_effect_ctrl.energy_total_data_record, (sizeof(light_effect_ctrl.energy_total_data_record)/sizeof(light_effect_ctrl.energy_total_data_record[0])));

	//printf("==========variance:%lf standarddeviation:%lf energy_total:%d energy_total_average:%d============\n",variance,sqrt(variance),light_effect_ctrl.energy_total, light_effect_ctrl.energy_total_average);


	if ((light_effect_ctrl.energy_total  > light_effect_ctrl.energy_total_average))
		light_effect_ctrl.light_effect_drew = 1;
	else
		light_effect_ctrl.light_effect_drew = 2;
}

void get_rgb_form_color_wheel(int setp, int *r, int *g, int *b)
{

	*r = color_wheel[light_effect_ctrl.colors_wheel_index][0];
	*g = color_wheel[light_effect_ctrl.colors_wheel_index][1];
	*b = color_wheel[light_effect_ctrl.colors_wheel_index][2];

	light_effect_ctrl.colors_wheel_index = light_effect_ctrl.colors_wheel_index + setp;
	if (light_effect_ctrl.colors_wheel_index >= (sizeof(color_wheel)/sizeof(color_wheel[0])))
		light_effect_ctrl.colors_wheel_index = 0;
}

void *pbox_light_effect_drew(void *arg)
{
	int end = 0;
	int r,g,b;

	while (true) {

		if (!light_effect_ctrl.light_effect_drew ) {
			msleep(10);
			continue;
		}
		if ((light_effect_ctrl.light_effect_drew == 1)) {
			end = 6;
			for (int j = 0; j < end; j++) {
				get_rgb_form_color_wheel(5, &r, &g, &b);
				userspace_set_led_color(j, r*(end-j)/end, g*(end-j)/end, b*(end-j)/end);
				userspace_set_led_color(11 - j, r*(end-j)/end, g*(end-j)/end, b*(end-j)/end);
				msleep(20);
			}

		} else if (light_effect_ctrl.light_effect_drew == 2) {
			get_rgb_form_color_wheel(5, &r, &g, &b);
			end = rand()%2;
			for (int j = 6; j > end; j--) {
				userspace_set_led_color(j, 0, 0, 0);
				userspace_set_led_color(11 - j, 0, 0, 0);
				msleep(10);
			}
			userspace_set_led_color(end - 1, r/2, g/2, b/2);
			userspace_set_led_color(12-end, r/2, g/2, b/2);
		}

		light_effect_ctrl.light_effect_drew = 0;
	}
}

void pbox_light_effect_soundreactive(energy_data_t energy_data)
{
	if (!light_effect_ctrl.ctrl_mode) {
		set_led_status(RK_ECHO_LED_OFF);
	}
	pbox_light_effect_soundreactive_1(energy_data);
	light_effect_ctrl.ctrl_mode = 1;
}

static void *pbox_light_effect_server(void *arg)
{
	int light_effect_fds[1] = {0};
	int maxfd;
	char buff[sizeof(pbox_light_effect_msg_t)] = {0};
	pbox_light_effect_msg_t *msg;


	pthread_setname_np(pthread_self(), "party_light_effect");

	int sock_fd = create_udp_socket(SOCKET_PATH_LED_EFFECT_SERVER);

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
			printf("select timeout or no data\n");
			continue;
		}

		int ret = recvfrom(sock_fd, buff, sizeof(buff), 0, NULL, NULL);
		if (ret <= 0)
			continue;

		pbox_light_effect_msg_t *msg = (pbox_light_effect_msg_t *)buff;
		//printf("%s recv: type: %d, id: %d\n", __func__, msg->type, msg->msgId);

		if(msg->type == PBOX_EVT)
			continue;

		switch (msg->msgId) {
			case PBOX_LIGHT_EFFECT_SOUNDREACTIVE_EVT:
				pbox_light_effect_soundreactive(msg->energy_data);
				break;
			case RK_ECHO_SYSTEM_BOOTING_EVT:
				set_led_status(RK_ECHO_SYSTEM_BOOTING);
				break;
			case RK_ECHO_SYSTEM_BOOTC_EVT:
				set_led_status(RK_ECHO_SYSTEM_BOOTC);
				break;
			case RK_ECHO_NET_CONNECT_RECOVERY_EVT:
				set_led_status(RK_ECHO_NET_CONNECT_RECOVERY);
				break;
			case RK_ECHO_NET_CONNECT_WAITTING_EVT:
				set_led_status(RK_ECHO_NET_CONNECT_WAITTING);
				break;
			case RK_ECHO_NET_CONNECTING_EVT:
				set_led_status(RK_ECHO_NET_CONNECTING);
				break;
			case RK_ECHO_NET_CONNECT_FAIL_EVT:
				set_led_status(RK_ECHO_NET_CONNECT_FAIL);
				break;
			case RK_ECHO_NET_CONNECT_SUCCESS_EVT:
				set_led_status(RK_ECHO_NET_CONNECT_SUCCESS);
				break;
			case RK_ECHO_WAKEUP_WAITTING_EVT:
				set_led_status(RK_ECHO_WAKEUP_WAITTING);
				break;
			case RK_ECHO_TTS_THINKING_EVT:
				set_led_status(RK_ECHO_TTS_THINKING);
				break;
			case RK_ECHO_TTS_PLAYING_EVT:
				set_led_status(RK_ECHO_TTS_PLAYING);
				break;
			case RK_ECHO_BT_PAIR_WAITTING_EVT:
				set_led_status(RK_ECHO_BT_PAIR_WAITTING);
				break;
			case RK_ECHO_BT_PAIRING_EVT:
				set_led_status(RK_ECHO_BT_PAIRING);
				break;
			case RK_ECHO_BT_PAIR_FAIL_EVT:
				set_led_status(RK_ECHO_BT_PAIR_FAIL);
				break;
			case RK_ECHO_BT_PAIR_SUCCESS_EVT:
				set_led_status(RK_ECHO_BT_PAIR_SUCCESS);
				break;
			case RK_ECHO_VOLUME_LED_EVT:
				set_led_status(RK_ECHO_VOLUME_LED);
				break;
			case RK_ECHO_MIC_MUTE_EVT:
				set_led_status(RK_ECHO_MIC_MUTE);
				break;
			case RK_ECHO_MIC_UNMUTE_EVT:
				set_led_status(RK_ECHO_MIC_UNMUTE);
				break;
			case RK_ECHO_ALARM_EVT:
				set_led_status(RK_ECHO_ALARM);
				break;
			case RK_ECHO_UPGRADING_EVT:
				set_led_status(RK_ECHO_UPGRADING);
				break;
			case RK_ECHO_UPGRADE_END_EVT:
				set_led_status(RK_ECHO_UPGRADE_END);
				break;
			case RK_ECHO_LED_OFF_EVT:
				set_led_status(RK_ECHO_LED_OFF);
				break;
			case RK_ECHO_CHARGER_EVT:
				set_led_status(RK_ECHO_CHARGER);
				break;
			case RK_ECHO_LOW_BATTERY_EVT:
				set_led_status(RK_ECHO_LOW_BATTERY);
				break;
			case RK_ECHO_PHONE_EVT:
				set_led_status(RK_ECHO_PHONE);
				break;
			case RK_ECHO_TIME_EVT:
				set_led_status(RK_ECHO_TIME);
				break;
			case RK_ECHO_TEST_EVT:
				set_led_status(RK_ECHO_TEST);
				break;
			case RK_ECHO_PLAY_EVT:
				set_led_status(RK_ECHO_PLAY);
				break;
			case RK_ECHO_PAUSE_EVT:
				set_led_status(RK_ECHO_PAUSE);
				break;
			default: {
				 } break;
		}
	}
}

int pbox_light_effect_init(void)
{
	leds_multi_init();
	led_userspace_ctrl_init(36);
	light_effect_ctrl.ctrl_mode = 0;
}

pthread_t light_effect_task_id;
pthread_t light_effect_drew_id;

int pbox_create_lightEffectTask(void)
{
	int ret;

	pbox_light_effect_init();

	ret = pthread_create(&light_effect_task_id, NULL, pbox_light_effect_server, NULL);
	if (ret < 0)
	{
		printf("light effect server start failed\n");
	}

	ret = pthread_create(&light_effect_drew_id, NULL, pbox_light_effect_drew, NULL);
	if (ret < 0)
	{
		printf("light effect drew start failed\n");
	}

	return ret;

}

int pbox_light_effect_send_cmd(pbox_light_effect_opcode_t command, void *data, int len)
{
	pbox_light_effect_msg_t msg = {0};

	msg.type = RK_LIGHT_EFFECT_CMD;
	msg.msgId = command;
	if(data != NULL)
		memcpy(&msg.energy_data, data, len);

	unix_socket_send_cmd(PBOX_CHILD_LED,&msg, sizeof(pbox_light_effect_msg_t));
}
