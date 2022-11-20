#include "crypto.h"

namespace ns3 {

int Hex2Int(char c) {
    return (c >= '0' && c <= '9') ? (c) - '0' :
           (c >= 'A' && c <= 'F') ? (c) - 'A' + 10 :
           (c >= 'a' && c <= 'f') ? (c) - 'a' + 10 :
                                    0;
}

void Hex2Bin(const unsigned char *hex, int sz, unsigned char *out) {
    int i;
    for (i = 0; i < sz; i += 2) {
        out[i / 2] = (Hex2Int(hex[i]) << 4) | Hex2Int(hex[i + 1]);
    }
}

char *Bin2Hex(unsigned char *src, int size) {
    char *encodedHex = new char[size * 2 + 1];
    for (int i = 0; i < size; i++) {
        sprintf(&encodedHex[i * 2], "%02x", src[i]);
    }
    return encodedHex;
}

// ---- sha256摘要哈希 ---- //
std::string sha256(const std::string &srcStr) {
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, srcStr.c_str(), srcStr.size());
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &sha256); // 得到 2进制格式的sha256摘要, 转为 16进制表示
    char *encodedHex = Bin2Hex(hash, SHA256_DIGEST_LENGTH);
    encodedHex[32]   = '\0'; // 截断, 只取前32位
    std::string encodedHexStr(encodedHex);
    return encodedHexStr;
}

std::string generateRSASign(const std::string &digest, RSA *priKey) {
    unsigned int size = RSA_size(priKey);
    std::vector<char> sign;
    sign.resize(size);
    int ret =
        RSA_sign(NID_sha256, (const unsigned char *)digest.c_str(),
                 digest.size(), (unsigned char *)sign.data(), &size, priKey);
    // RSA_free(priKey);
    if (ret != 1) {
        std::cout << "RSA_sign failed" << std::endl;
        return "";
    }
    // 二进制转换为十六进制
    char *encodedHex = Bin2Hex((unsigned char *)sign.data(), size);
    std::string signature(encodedHex);
    return signature;
}

bool verifyRSASign(std::string &digest, RSA *pubKey, std::string &signature) {
    std::vector<char> sign;
    sign.resize(signature.size() / 2);
    Hex2Bin((unsigned char *)signature.c_str(), signature.size(), (unsigned char *)sign.data());
    int ret =
        RSA_verify(NID_sha256, (const unsigned char *)digest.c_str(),
                   digest.size(), (unsigned char *)sign.data(), sign.size(), pubKey);
    // RSA_free(pubKey);
    return ret == 1;
}

bool checkRSAKeyPair(int nodeId) {
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
    return std::filesystem::exists(publicFile) && std::filesystem::exists(privateFile);
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

RSA *getPubKey(int nodeId) {
    std::string publicFile = KeysPath + "/node" + std::to_string(nodeId) + "/public.pem";
    OpenSSL_add_all_algorithms();
    BIO *in = BIO_new(BIO_s_file());
    if (in == NULL) {
        std::cout << "BIO_new failed" << std::endl;
        return RSA_new();
    }
    BIO_read_filename(in, publicFile.c_str());
    RSA *pubKey = PEM_read_bio_RSAPublicKey(in, NULL, NULL, NULL);
    if (pubKey == NULL) {
        std::cout << "node" << nodeId << " 获取公钥失败..." << std::endl;
        return RSA_new();
    }
    BIO_free(in);
    return pubKey;
}

RSA *getPriKey(int nodeId) {
    std::string privateFile = KeysPath + "/node" + std::to_string(nodeId) + "/private.pem";
    OpenSSL_add_all_algorithms();
    BIO *in = BIO_new(BIO_s_file());
    if (in == NULL) {
        std::cout << "BIO_new failed" << std::endl;
        return RSA_new();
    }
    BIO_read_filename(in, privateFile.c_str());
    RSA *priKey = PEM_read_bio_RSAPrivateKey(in, NULL, NULL, NULL);
    BIO_free(in);
    if (priKey == NULL) {
        std::cout << "node" << nodeId << " 获取私钥失败..." << std::endl;
        return RSA_new();
    }
    return priKey;
}

// void getPubKey(int nodeId, RSA *pubKey) {
//     std::string publicFile = KeysPath + "/node" + std::to_string(nodeId) + "/public.pem";
//     BIO *pub               = BIO_new_file(publicFile.c_str(), "r");
//     if (pub == NULL) {
//         std::cout << "node" << nodeId << " 读取公钥文件失败..." << std::endl;
//         return;
//     }
//     pubKey = PEM_read_bio_RSAPublicKey(pub, &pubKey, NULL, NULL);
//     if (!pubKey) {
//         BIO_free_all(pub);
//         std::cout << "node" << nodeId << " 获取公钥失败..." << std::endl;
//         return;
//     }
//     // int length = RSA_size(pubKey);
//     // std::cout << "公钥长度: " << length << std::endl;
//     BIO_free_all(pub);
// }

// void getPriKey(int nodeId, RSA *priKey) {
//     std::string privateFile = KeysPath + "/node" + std::to_string(nodeId) + "/private.pem";
//     BIO *pri                = BIO_new_file(privateFile.c_str(), "r");
//     if (pri == NULL) {
//         std::cout << "node" << nodeId << " 读取私钥文件失败..." << std::endl;
//         return;
//     }
//     priKey = PEM_read_bio_RSAPrivateKey(pri, &priKey, NULL, NULL);
//     if (!priKey) {
//         BIO_free_all(pri);
//         std::cout << "node" << nodeId << " 获取私钥失败..." << std::endl;
//         return;
//     }
//     // int length = RSA_size(priKey);
//     // std::cout << "私钥长度: " << length << std::endl;
//     BIO_free_all(pri);
// }
} // namespace ns3
