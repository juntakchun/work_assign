#include <iostream>
#include <fstream>
#include <string>
#include <typeinfo>
#include <map>
#include <vector>
#include <ctime>

class Driver {
private:
	int id;
	std::string name;
	int status;			// 1: Ld, 2: Md, 3: Gd
	std::vector<int> limitDate;
	
	// # of works in each category
	int numDang = 0;
	int numGit = 0;
	int numUl = 0;
	int numSun = 0;
	

public:
	Driver(int _id, std::string& _name, int _status) : id(_id), status(_status)
	{
		name = _name;
	}
	
	void printDriverInfo() {
		std::string statusName;
		switch (status) {
			case 1:
				statusName = "대형운전";
				break;
			case 2:
				statusName = "중형운전";
				break;
			case 3:
				statusName = "대형계원";
				break;
			default:
				break;
		}
		std::cout << id << " " << statusName << " " << name << " Ul:" << numUl << " Dang: " << numDang << " Git:" << numGit << std::endl;
	}
	
	const int getId() { return id;}
	const int getStatus() { return status; }
	const int getNumUl() { return numUl; }
	const int getNumDang() { return numDang; }
	const int getNumGit() { return numGit; }

	void addNumUl() { numUl++; }
	void addNumGit() { numGit++; }
	void addNumDang() { numDang++; }
};


class WorkHandler {
private:
	Driver* drvArr[30];
	int drvNum;				// # of drivers currently exist
	int BdrvNum;
	int MdrvNum;
	int GdrvNum;
	
	int calendar[35][4];	// Ul:1, Dang:2, Git:3
	/*	index of calendar
	S	M	T	W	T	F	S
	0	1	2	3	4	5	6
	7	8	9	10	11	12	13
	14	15	16	17	18	19	20
	21	22	23	24	25	26	27	
	28	29	30	31	32	33	34
	*/
	
	std::map<int, std::string> id_name_map;
	
public:
	WorkHandler() : drvNum(0) { }
	~WorkHandler() {
		for (auto i = 0; i < drvNum; i++) {
			delete drvArr[i];
		}
	}
	
	void AddDriver(int _id, std::string& _name, int _status);
	void DeleteDriver();
	void ShowDriver();
	
	void FixGit(int* driverList);
	void FixUl();
	void FixDang();
	
	void printCalendar();
	
	void ReadDriverList(const char* filename);
	void WriteWorkSchedule();

	void PrintDriverList();
};

std::string CurrentTime();
	
int main () {
	
	WorkHandler hdr;
	hdr.ReadDriverList("./driverlist.csv");
	// hdr.PrintDriverList();
	
	int gitDriverList[5] = {102, 201, 101, 204, 103};
	hdr.FixGit(gitDriverList);
	hdr.FixUl();
	hdr.FixDang();
	hdr.printCalendar();
	hdr.PrintDriverList();
	
	hdr.WriteWorkSchedule();

}


void WorkHandler::AddDriver(int _id, std::string& _name, int _status) {
	drvArr[drvNum++] = new Driver(_id, _name, _status);
}

void WorkHandler::FixGit(int* driverList) {
	for (auto i=0; i<5; i++) {
		auto targetDrv = driverList[i];
		for (auto j=0; j<7; j++) {
			calendar[i*7 + j][3] = targetDrv;
		}
		// std::cout << calendar[i*7][3] << std::endl;
		for (auto i = 0; i < drvNum; i++) {
			if(drvArr[i]->getId() == targetDrv)
				drvArr[i]->addNumGit();
		}
	}
	
}

void WorkHandler::FixUl() {
	
	/*	Rule
	
	1. list 순서대로 뿌리기
	2. 기타대 인원 제외	
	3. 주말 횟수 고르게
	
	*/

	int drvIdx = 0;	
	int maxUl = 5;
	for (auto i = 0; i < 35; i++) {	
		while(true) {
			drvIdx = drvIdx % drvNum;
			if (drvArr[drvIdx]->getStatus() != 2) { // 중형 거르기
				if (drvArr[drvIdx]->getNumUl() < maxUl) { // 과다 근무자 거르기
					if (calendar[i][3] != drvArr[drvIdx]->getId()) { // 기타대 근무자 거르기
						if (i % 7 == 5) {
							if (i == 33) break;
							else if (calendar[i+7][3] != drvArr[drvIdx]->getId()) { // 금요일
								break;
							}
						}
						else if (i % 7 == 6) {
							if (i == 34) break;
							else if (calendar[i+7][3] != drvArr[drvIdx]->getId()) { // 토요일
								break;
							}
						}
						else
							break;
					}
					else if (i % 7 == 6) {
						break;
					}
				}
			}		
			drvIdx++;
		}
		
		// std::cout << i << " " << id_name_map.find(drvArr[drvIdx]->getId())->second << std::endl;
		
		calendar[i][1] = drvArr[drvIdx]->getId();
		drvArr[drvIdx]->addNumUl();
		
		drvIdx++;
	}
	
}

void WorkHandler::FixDang() {
	
	/*	Rule
	
	1. list 순서대로 뿌리기
	2. 기타대 & 울타리 인원 제외	
	3. 주말 횟수 고르게(?) + 대형/중형 차별화
	4. 당근당근
	
	*/
	
	int drvIdx = 0;	
	int maxDang_M = 10;
	int maxDang_L = 1;
	
	for (auto i = 0; i < 35; i++) {	
		while(true) {
			drvIdx = drvIdx % drvNum;
			
			if (i <= 33) {
				if (calendar[i][1] != drvArr[drvIdx]->getId() && calendar[i+1][1] != drvArr[drvIdx]->getId()) { // 울타리 근무자 거르기
					if ((drvArr[drvIdx]->getStatus() == 2 && drvArr[drvIdx]->getNumDang() < maxDang_M)
					   || (drvArr[drvIdx]->getStatus() != 2 && drvArr[drvIdx]->getNumDang() < maxDang_L)) { // 과다 근무자 거르기
						if (i > 1) { // 당근당근 피하기
							if (calendar[i-2][2] != drvArr[drvIdx]->getId() && calendar[i-1][2] != drvArr[drvIdx]->getId()) {
								if (calendar[i][3] != drvArr[drvIdx]->getId()) { // 기타대 근무자 거르기
									if (i % 7 == 5) {
										if (i == 33) break;
										else if (calendar[i+7][3] != drvArr[drvIdx]->getId()) { // 금요일
											break;
										}
									}
									else if (i % 7 == 6) {
										if (i == 34) break;
										else if (calendar[i+7][3] != drvArr[drvIdx]->getId()) { // 토요일
											break;
										}
									}
									else
										break;
								}
								else if (i % 7 == 6) {
									break;
								}
								
							}
						}
						else {
							if (calendar[i][3] != drvArr[drvIdx]->getId()) { // 기타대 근무자 거르기
								if (i % 7 == 5) {
									if (i == 33) break;
									else if (calendar[i+7][3] != drvArr[drvIdx]->getId()) { // 금요일
										break;
									}
								}
								else if (i % 7 == 6) {
									if (i == 34) break;
									else if (calendar[i+7][3] != drvArr[drvIdx]->getId()) { // 토요일
										break;
									}
								}
								else
									break;
							}
							else if (i % 7 == 6) {
								break;
							}
						}
					} 
				}
			}
			else {
				break;
			}
			drvIdx++;
		}
		
		// std::cout << i << " " << id_name_map.find(drvArr[drvIdx]->getId())->second << std::endl;
		
		calendar[i][2] = drvArr[drvIdx]->getId();
		drvArr[drvIdx]->addNumDang();
		
		drvIdx++;
	}
	
}

void WorkHandler::printCalendar() {
	
	std::cout << "\t\t\t    Calendar"<< std::endl << std::endl;
	std::cout << "\tS\tM\tT\tW\tT\tF\tS" << std::endl << std::endl;;
	for (auto i=0; i<5; i++) {
		for (auto j=0; j<7; j++) {
			std::cout << "\t" << (i*7 + j);
		}
		std::cout << std::endl;
		std::cout << "U\t";
		for (auto j=0; j<7; j++) {
			std::cout << id_name_map.find(calendar[i*7+j][1])->second << "\t";
		}
		std::cout << std::endl;
		std::cout << "D\t";
		for (auto j=0; j<7; j++) {
			std::cout << id_name_map.find(calendar[i*7+j][2])->second << "\t";
		}
		std::cout << std::endl;
		
		std::cout << "G\t\t\t\t" << id_name_map.find(calendar[i*7][3])->second << std::endl << std::endl;
	}
	
	// for (auto i = 0; i < 35; i++) {
	// 	std::cout << calendar[i][1] << std::endl;
	// }
}

void WorkHandler::ReadDriverList(const char* filename) {
	std::string str_buf;
	std::string delimiter(",");
	std::fstream fs;
	fs.open(filename, std::ios::in);
	
	while(!fs.eof()) {
		std::getline(fs, str_buf);
		auto pos = str_buf.find(delimiter);
		
		// get id
		std::string token = str_buf.substr(0, pos);
		auto id = std::stoi(token);
		str_buf.erase(0, pos + delimiter.length());
		
		// get name
		pos = str_buf.find(delimiter);
		token = str_buf.substr(0, pos);
		auto name = token;
		str_buf.erase(0, pos + delimiter.length());
		
		// get status
		auto status = std::stoi(str_buf);
		
		AddDriver(id, name, status);
		id_name_map.insert({id, name});
	}
	
	fs.close();
}

void WorkHandler::WriteWorkSchedule() {
	std::ofstream os;
	std::string filename("./workSchedule_" + CurrentTime() + ".csv");
	std::cout << filename;
	os.open(filename);
	
	os << ",,,,Calendar\n\n";
	os << ",Sun,Mon,Tue,Wed,Thu,Fri,Sat\n\n";
	
	for (auto i=0; i<5; i++) {
		for (auto j=0; j<7; j++) {
			os << "," << (i*7 + j);
		}
		os << "\n";
		os << "U,";
		for (auto j=0; j<7; j++) {
			os << id_name_map.find(calendar[i*7+j][1])->second << ",";
		}
		os << "\n";
		os << "D,";
		for (auto j=0; j<7; j++) {
			os << id_name_map.find(calendar[i*7+j][2])->second << ",";
		}
		os << "\n";
		
		os << "G,,,," << id_name_map.find(calendar[i*7][3])->second << "\n\n";
	}
	
	os << "\n\n\n";
	
	os << "D,";
	for (auto i=0; i<drvNum; i++) {
		os << id_name_map.find(drvArr[i]->getId())->second << ",";
	}
	os << "\n,";
	for (auto i=0; i<drvNum; i++) {
		os << drvArr[i]->getNumDang() << ",";
	}
	os << "\n\n";
	
	os << "U,";
	for (auto i=0; i<drvNum; i++) {
		if (drvArr[i]->getStatus() != 2)
			os << id_name_map.find(drvArr[i]->getId())->second << ",";
	}
	os << "\n,";
	for (auto i=0; i<drvNum; i++) {
		if (drvArr[i]->getStatus() != 2)
			os << drvArr[i]->getNumUl() << ",";
	}
	os << "\n\n";
	
	os << "G,";
	for (auto i=0; i<drvNum; i++) {
		if (drvArr[i]->getNumGit() != 0)
			os << id_name_map.find(drvArr[i]->getId())->second << ",";
	}
	os << "\n,";
	for (auto i=0; i<drvNum; i++) {
		if (drvArr[i]->getNumGit() != 0)
			os << "1" << ",";
	}
	
	os.close();
}

void WorkHandler::PrintDriverList() {
	for (auto i = 0; i < drvNum; i++) {
		drvArr[i]->printDriverInfo();
	}
	
	
	// for (auto iter : id_name_map) {
	// 	std::cout << iter.first << " " << iter.second << std::endl;
	// }
}

std::string CurrentTime() {
	struct tm curr_tm;
    time_t curr_time = time(nullptr);

    localtime_r(&curr_time, &curr_tm);

    int curr_year = curr_tm.tm_year + 1900 - 2000;
    int curr_month = curr_tm.tm_mon + 1;
    int curr_day = curr_tm.tm_mday;
    int curr_hour = curr_tm.tm_hour;
    int curr_minute = curr_tm.tm_min;
    int curr_second = curr_tm.tm_sec;
	
	std::string time = std::to_string(curr_year) + std::to_string(curr_month) + std::to_string(curr_day)
		+ std::to_string(curr_hour) + std::to_string(curr_minute) + std::to_string(curr_second);
		
	return time;
}