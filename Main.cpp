#include <iostream>
#include <chrono>
#define UNICODE
#include <Windows.h>
#include <math.h>
#include <vector>
#include <algorithm>
using namespace std;


int screenWidth = 120;
int screenHeight = 40;

float playerX = 8.f;
float playerY = 8.f;
float playerAngle = 0.f;

int mapHeight = 16;
int mapWidth = 16;

float fov = 3.14159 / 4;
float depth = 16.0f;

int main() {
	wchar_t* screen = new wchar_t[screenWidth * screenHeight];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;


	wstring map;
	map += L"################";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..........#...#";
	map += L"#..........#...#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#.......########";
	map += L"#..............#";
	map += L"#..............#";
	map += L"################";


	auto timePoint1 = chrono::system_clock::now();
	auto timePoint2 = chrono::system_clock::now();

	while (true) {
		timePoint2 = chrono::system_clock::now();
		chrono::duration<float> elapsedTimeDuration = timePoint2 - timePoint1;
		timePoint1 = timePoint2;
		float elapsedTime = elapsedTimeDuration.count();

		// controls
		if (GetAsyncKeyState((unsigned short)'A') & 0x8000) {
			playerAngle -= (1.0f) * elapsedTime;
		}

		if (GetAsyncKeyState((unsigned short)'D') & 0x8000) {
			playerAngle += (1.0f) * elapsedTime;
		}

		if (GetAsyncKeyState((unsigned short)'W') & 0x8000) {
			playerX += sinf(playerAngle) * 5.0f * elapsedTime;
			playerY += cosf(playerAngle) * 5.0f * elapsedTime;

			if (map[(int)playerY * mapWidth + (int)playerX] == '#') {
				playerX -= sinf(playerAngle) * 5.0f * elapsedTime;
				playerY -= cosf(playerAngle) * 5.0f * elapsedTime;
			}
		}

		if (GetAsyncKeyState((unsigned short)'S') & 0x8000) {
			playerX -= sinf(playerAngle) * 5.0f * elapsedTime;
			playerY -= cosf(playerAngle) * 5.0f * elapsedTime;

			if (map[(int)playerY * mapWidth + (int)playerX] == '#') {
				playerX += sinf(playerAngle) * 5.0f * elapsedTime;
				playerY += cosf(playerAngle) * 5.0f * elapsedTime;
			}
		}

		for (int x = 0; x < screenWidth; x++) {
			float rayAngle = (playerAngle - fov / 2.0f) + ((float)x / (float)screenWidth) * fov;

			float distanceToWall = 0;
			bool hitWall = false;
			bool boundary = false;

			float eyeX = sinf(rayAngle); // unit vector for ray in player space
			float eyeY = cosf(rayAngle);

			while (!hitWall && distanceToWall < depth) {
				distanceToWall += 0.1f;

				int testX = (int)(playerX + eyeX * distanceToWall);
				int testY = (int)(playerY + eyeY * distanceToWall);

				// is ray out of bounds?
				if (testX < 0 || testX >= mapWidth || testY < 0 || testY >= mapHeight) {
					hitWall = true;
					distanceToWall = depth;
				}
				else {
					// ray is in bounds so see if the ray cell is a wall
					if (map[testY * mapWidth + testX] == '#') {
						hitWall = true;
						vector<pair<float, float>> p; // distance, dot

						for (int tx = 0; tx < 2; tx++) {
							for (int ty = 0; ty < 2; ty++) {
								float vx = (float)testX + tx - playerX;
								float vy = (float)testY + ty - playerY;

								float d = sqrt(vx * vx + vy * vy);
								float dot = (eyeX * vx / d) + (eyeY * vy / d);
								p.push_back(make_pair(d, dot));
							}
						}
						
						// sort pairs from closest to farthest
						sort(p.begin(), p.end(), [](const pair<float, float>& left, const pair<float, float>& right) {return left.first < right.first; });

						float bound = 0.01;
						if (acos(p.at(0).second) < bound) {
							boundary = true;
						}
						if (acos(p.at(1).second) < bound) {
							boundary = true;
						}
						if (acos(p.at(2).second) < bound) {
							boundary = true;
						}
					}
				}
			}

			// find distance to ceiling and floor
			int ceiling = (float)(screenHeight / 2.0) - screenHeight / ((float)distanceToWall);
			int floor = screenHeight - ceiling;

			short shade = ' ';

			if (distanceToWall <= depth / 4.0f) {
				shade = 0x2588; // very close
			}
			else if (distanceToWall < depth / 3.0f) {
				shade = 0x2593;
			}
			else if (distanceToWall < depth / 2.0f) {
				shade = 0x2592;
			}
			else if (distanceToWall < depth) {
				shade = 0x2591;
			}
			else {
				shade = ' '; // far away	
			}

			if (boundary) {
				shade = ' ';
			}

			for (int y = 0; y < screenHeight; y++) {
				if (y < ceiling) {
					screen[y * screenWidth + x] = ' ';
				}
				else if (y > ceiling && y <= floor) {
					screen[y * screenWidth + x] = shade;
				}
				else {
					short floorShade = ' ';
					float b = 1.0f - (((float)y - screenHeight / 2.0f) / ((float)screenHeight / 2.0f));
					if (b < 0.25f) {
						floorShade = '#'; // very close
					}
					else if (b < 0.5f) {
						floorShade = 'x';
					}
					else if (b < 0.75f) {
						floorShade = '.';
					}
					else if (b < 0.9f) {
						floorShade = '-';
					}
					else {
						floorShade = ' '; // far away	
					}
					screen[y * screenWidth + x] = floorShade;
				}
			}
		}

		// display stats
		swprintf(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f, FPS=%3.2f ", playerX, playerY, playerAngle, 1.0f / elapsedTime);

		// display map
		for (int nx = 0; nx < mapWidth; nx++) {
			for (int ny = 0; ny < mapWidth; ny++) {
				screen[(ny + 1) * screenWidth + nx] = map[ny * mapWidth + nx];
			}
		}

		screen[((int)playerY + 1) * screenWidth + (int)playerX] = 'P';

		screen[screenWidth * screenHeight - 1] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, screenWidth * screenHeight, { 0,0 }, &dwBytesWritten);
	}

	return 0;
}