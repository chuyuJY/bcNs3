#ifndef CRYPTO_H
#define CRYPTO_H
#include <algorithm>
#include <fstream>
#include <bits/stdc++.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include "openssl/des.h"
#include "openssl/rsa.h"
#include "openssl/pem.h"

#define KEY_LENGTH 1024 // 密钥长度
const std::string KeysPath = "keys";

namespace ns3 {
int Hex2Int(char c);

void Hex2Bin(const unsigned char *hex, int sz, unsigned char *out);

char *Bin2Hex(unsigned char *src, int size);

std::string sha256(const std::string &srcStr);

bool checkRSAKeyPair(int nodeId); // 检查该节点是否已有RSA密钥, 若无, 自动创建

void generateRSAKey(std::string publicFile, std::string privateFile); // 生成RSA到指定文件目录

std::string generateRSASign(const std::string &digest, RSA *priKey);

bool verifyRSASign(std::string &digest, RSA *pubKey, std::string &signature);

RSA *getPubKey(int nodeId);

RSA *getPriKey(int nodeId);

} // namespace ns3

#endif // CRYPTO_H