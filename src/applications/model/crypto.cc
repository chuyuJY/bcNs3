#include "crypto.h"

namespace ns3 {

// ---- sha256摘要哈希 ---- //
std::string sha256(const std::string &srcStr) {
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, srcStr.c_str(), srcStr.size());
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &sha256); // 得到 2进制格式的sha256摘要, 转为 16进制表示
    char *encodedHex = new char[SHA256_DIGEST_LENGTH * 2 + 1];
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(&encodedHex[i * 2], "%02x", hash[i]);
    }
    encodedHex[32] = '\0'; // 截断, 只取前32位
    std::string encodedHexStr(encodedHex);
    return encodedHexStr;
}

std::string sign(std::string &digest, RSA *priKey) {
    std::cout << priKey << std::endl;
    unsigned int size = RSA_size(priKey);
    // std::cout << "digest.data()为: " << digest.data() << std::endl;
    // unsigned char *out  = new unsigned char[length];
    // unsigned int outLen = 0;
    std::vector<char> out;
    out.resize(size);
    int ret =
        RSA_sign(NID_sha256, (const unsigned char *)digest.data(),
                 digest.size(), (unsigned char *)out.data(), &size, priKey);
    if (ret != 1) {
        std::cout << "RSA_sign failed" << std::endl;
        return "";
    }
    // std::cout << "outLen为: " << outLen << std::endl;
    // std::string signature((char *)out, outLen);
    std::string signature(out.begin(), out.end());
    return signature;
}

bool verifySign(std::string &digest, RSA *pubKey, std::string &signature) {
    int ret = RSA_verify(NID_sha256, (const unsigned char *)digest.data(), digest.size(),
                         (const unsigned char *)signature.data(), signature.size(), pubKey);
    std::cout << "验证签名输出值为: " << ret << std::endl;
    // ret == 1 为验证成功
    return ret == 1;
}

void getRSAKeyPair(int nodeId, RSA *pubKey, RSA *priKey) {
    std::string filePath    = KeysPath + "/node" + std::to_string(nodeId);
    std::string publicFile  = filePath + "/public.pem";
    std::string privateFile = filePath + "/private.pem";
    // 检查该节点是否已有公私钥对
    if (!std::filesystem::exists(publicFile) || !std::filesystem::exists(privateFile)) {
        if (!std::filesystem::exists(filePath)) {
            std::filesystem::create_directory(filePath);
        }
        generateRSAKey(publicFile, privateFile);
    }
    // 从文件读取密钥对
    getPubKey(nodeId, pubKey);
    getPriKey(nodeId, priKey);
}

void generateRSAKey(std::string publicFile, std::string privateFile) {
    RSA *keyPair = RSA_new();
    BIGNUM *e    = BN_new();
    BN_set_word(e, RSA_F4);
    RSA_generate_key_ex(keyPair, KEY_LENGTH, e, NULL);
    BIO *pri = BIO_new_file(privateFile.c_str(), "w");
    BIO *pub = BIO_new_file(publicFile.c_str(), "w");
    PEM_write_bio_RSAPublicKey(pub, keyPair);
    PEM_write_bio_RSAPrivateKey(pri, keyPair, NULL, NULL, 0, NULL, NULL);
    // 释放资源
    RSA_free(keyPair);
    BIO_free(pri);
    BIO_free(pub);
}

void getPubKey(int nodeId, RSA *pubKey) {
    std::string publicFile = KeysPath + "/node" + std::to_string(nodeId) + "/public.pem";
    BIO *pub               = BIO_new_file(publicFile.c_str(), "r");
    if (pub == NULL) {
        std::cout << "node" << nodeId << " 读取公钥文件失败..." << std::endl;
        return;
    }
    pubKey = PEM_read_bio_RSAPublicKey(pub, &pubKey, NULL, NULL);
    if (!pubKey) {
        BIO_free_all(pub);
        std::cout << "node" << nodeId << " 获取公钥失败..." << std::endl;
        return;
    }
    // int length = RSA_size(pubKey);
    // std::cout << "公钥长度: " << length << std::endl;
    BIO_free_all(pub);
}

void getPriKey(int nodeId, RSA *priKey) {
    std::string privateFile = KeysPath + "/node" + std::to_string(nodeId) + "/private.pem";
    BIO *pri                = BIO_new_file(privateFile.c_str(), "r");
    if (pri == NULL) {
        std::cout << "node" << nodeId << " 读取私钥文件失败..." << std::endl;
        return;
    }
    priKey = PEM_read_bio_RSAPrivateKey(pri, &priKey, NULL, NULL);
    if (!priKey) {
        BIO_free_all(pri);
        std::cout << "node" << nodeId << " 获取私钥失败..." << std::endl;
        return;
    }
    // int length = RSA_size(priKey);
    // std::cout << "私钥长度: " << length << std::endl;
    BIO_free_all(pri);
}
} // namespace ns3
