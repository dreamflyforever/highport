#include <iostream>
#include <fstream>
#include <vector>

// 加密函数
void encryptModel(const std::string& inputFile, const std::string& outputFile, const std::string& password) {
	std::ifstream inFile(inputFile, std::ios::binary);
	std::ofstream outFile(outputFile, std::ios::binary);

	char ch;
	size_t i = 0;
	while (inFile.get(ch)) {
		// 使用密码进行异或加密
		ch ^= password[i % password.length()];
		outFile.put(ch);
		i++;
	}

	inFile.close();
	outFile.close();

	std::cout << "模型已成功加密为：" << outputFile << std::endl;
}

// 解密函数
void decryptModel(const std::string& inputFile, const std::string& outputFile, const std::string& password) {
	std::ifstream inFile(inputFile, std::ios::binary);
	std::ofstream outFile(outputFile, std::ios::binary);

	char ch;
	size_t i = 0;
	while (inFile.get(ch)) {
		// 使用密码进行异或解密
		ch ^= password[i % password.length()];
		outFile.put(ch);
		i++;
	}

	inFile.close();
	outFile.close();

	std::cout << "模型已成功解密为：" << outputFile << std::endl;
}

int main() {
	std::string modelFile = "libknos.so";
	std::string encryptedModelFile = "libenos.so";
	std::string decryptedModelFile = "libdeos.so";

	std::string password = "myPassword123";  // 设置加密和解密的密码

	encryptModel(modelFile, encryptedModelFile, password);  // 加密模型文件

	// 在实际使用时，需要解密模型才能加载和使用
	decryptModel(encryptedModelFile, decryptedModelFile, password);  // 解密模型文件

	return 0;
}
