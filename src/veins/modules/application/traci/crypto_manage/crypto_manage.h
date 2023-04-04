#ifndef CRYPTO_MANAGE_H
#define CRYPTO_MANAGE_H
#include <cstdint>
#include <chrono>
#include <string>
class CryptoManage
{
public:
    virtual void initialize() = 0;
    //设置公钥，从文件读取公钥
    /*
    *   1. publickey_path  公钥文件路径
    *   成功返回 0
    *   失败返回非0
    */
    virtual int setPublicKey(const std::string& publickey_path) = 0;
    //设置公钥，从内存地址读取公钥
    /*
    *   1. pem_key_buffer       存放公钥的内存起始地址
    *   2. pem_key_buffer_len   缓冲区长度
    *   成功返回 0
    *   失败返回非0
    */
    virtual int setPublicKey(const unsigned char* pem_key_buffer, size_t pem_key_buffer_len = -1) = 0;
    //设置私钥，从文件读取私钥
    /*
    *   1. privatekey_path  私钥文件路径
    *   2. cipher           私钥的加密密码
    *   成功返回 0
    *   失败返回非0
    */
    virtual int setPrivateKey(const std::string& privatekey_path, const std::string& cipher = "") = 0;
    //设置私钥，从内存地址读取私钥
    /*
    *   1. pem_key_buffer       存放私钥的内存起始地址
    *   2. pem_key_buffer_len   缓冲区长度
    *   3. cipher               私钥的加密密码
    *   成功返回 0
    *   失败返回非0
    */
    virtual int setPrivateKey(const unsigned char* pem_key_buffer, size_t pem_key_buffer_len = -1, const unsigned char* cipher=NULL) = 0;
    //生成密钥对
    /*
    *   1. publickey_path  公钥文件输出路径
    *   2. privatekey_path 私钥文件输出路径
    *   3. cipher          私钥文件加密密码
    *   成功返回 0
    *   失败返回非0
    */
    virtual int generate_key(const std::string& publickey_path, const std::string& privatekey_path, const std::string& cipher = "") = 0;

    //用提供的公钥文件对明文进行加密，
    /*
    *   1. plainStr         需要加密的明文字符串
    *   2. publickey_path   加密明文的公钥文件路径
    *   成功返回加密密文的16进制字符串
    *   失败返回空串
    */
    virtual std::string encrypt(const std::string &plainStr, const std::string& publickey_path) = 0;

    //用已经设置好的公钥对明文进行加密，
    /*
    *   1. plainStr         需要加密的明文字符串
    *   成功返回加密密文的16进制字符串
    *   失败返回空串
    */
    virtual std::string encrypt(const std::string &plainStr) = 0;
    //用提供的私钥文件对密文进行解密，
    /*
    *   1. encryptedHexStr  需要解密的密文16进制字符串
    *   2. privatekey_path   用来解密密文的私钥文件路径
    *   成功返回解密后的明文字符串
    *   失败返回空串
    */
    virtual std::string decrypt(const std::string &encryptedHexStr, const std::string& privatekey_path, const std::string& cipher = "") = 0;

    //用已经设置好的私钥对密文进行解密，
    /*
    *   1. encryptedHexStr  需要解密的密文16进制字符串
    *   成功返回解密后的明文字符串
    *   失败返回空串
    */
    virtual std::string decrypt(const std::string &encryptedHexStr) = 0;
};

class CryptoManageFactory
{
public:
    virtual CryptoManage* createCryptoManage() = 0;
};

#endif 