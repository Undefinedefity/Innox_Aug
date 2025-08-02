#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
ESP32S3 连接测试脚本
用于测试与 ESP32S3 文件服务器的连接
"""

import requests
import json
import sys

def test_connection(ip_address):
    """测试与 ESP32S3 的连接"""
    base_url = f"http://{ip_address}"
    
    print(f"正在测试连接到: {base_url}")
    print("-" * 50)
    
    # 测试 1: 基本连接
    print("1. 测试基本连接...")
    try:
        response = requests.get(base_url, timeout=5)
        if response.status_code == 200:
            print("✅ 基本连接成功")
        else:
            print(f"⚠️  连接成功但状态码异常: {response.status_code}")
    except requests.exceptions.RequestException as e:
        print(f"❌ 基本连接失败: {e}")
        return False
    
    # 测试 2: 获取文件列表 (JSON)
    print("\n2. 测试文件列表 API...")
    try:
        response = requests.get(f"{base_url}/list", timeout=5)
        if response.status_code == 200:
            data = response.json()
            files = data.get('files', [])
            print(f"✅ 文件列表 API 正常，找到 {len(files)} 个文件")
            if files:
                print("文件列表:")
                for i, file_info in enumerate(files[:5], 1):  # 只显示前5个
                    print(f"  {i}. {file_info['name']} ({file_info['size']} 字节)")
                if len(files) > 5:
                    print(f"  ... 还有 {len(files) - 5} 个文件")
            else:
                print("⚠️  SD 卡中没有文件")
        else:
            print(f"❌ 文件列表 API 失败: {response.status_code}")
    except requests.exceptions.RequestException as e:
        print(f"❌ 文件列表 API 请求失败: {e}")
    except json.JSONDecodeError as e:
        print(f"❌ 文件列表 API 返回格式错误: {e}")
    
    # 测试 3: 获取 HTML 页面
    print("\n3. 测试 HTML 页面...")
    try:
        response = requests.get(base_url, timeout=5)
        if response.status_code == 200:
            content_type = response.headers.get('content-type', '')
            if 'text/html' in content_type:
                print("✅ HTML 页面正常")
                print(f"   内容类型: {content_type}")
                print(f"   页面大小: {len(response.text)} 字符")
            else:
                print(f"⚠️  页面内容类型异常: {content_type}")
        else:
            print(f"❌ HTML 页面请求失败: {response.status_code}")
    except requests.exceptions.RequestException as e:
        print(f"❌ HTML 页面请求失败: {e}")
    
    # 测试 4: 测试文件下载 (如果存在文件)
    print("\n4. 测试文件下载...")
    try:
        response = requests.get(f"{base_url}/list", timeout=5)
        if response.status_code == 200:
            data = response.json()
            files = data.get('files', [])
            
            if files:
                # 测试下载第一个文件
                test_file = files[0]['name']
                print(f"   测试下载文件: {test_file}")
                
                download_response = requests.get(f"{base_url}/file?name={test_file}", 
                                               timeout=10, stream=True)
                if download_response.status_code == 200:
                    content_length = download_response.headers.get('content-length', '未知')
                    content_type = download_response.headers.get('content-type', '未知')
                    print(f"✅ 文件下载测试成功")
                    print(f"   文件大小: {content_length} 字节")
                    print(f"   内容类型: {content_type}")
                else:
                    print(f"❌ 文件下载测试失败: {download_response.status_code}")
            else:
                print("⚠️  没有文件可供下载测试")
        else:
            print("❌ 无法获取文件列表进行下载测试")
    except requests.exceptions.RequestException as e:
        print(f"❌ 文件下载测试失败: {e}")
    
    print("\n" + "=" * 50)
    print("连接测试完成！")
    return True

def main():
    """主函数"""
    print("ESP32S3 连接测试工具")
    print("=" * 50)
    
    if len(sys.argv) > 1:
        ip_address = sys.argv[1]
    else:
        ip_address = input("请输入 ESP32S3 的 IP 地址: ").strip()
    
    if not ip_address:
        print("错误: 必须提供 IP 地址")
        sys.exit(1)
    
    # 移除可能的 http:// 前缀
    if ip_address.startswith("http://"):
        ip_address = ip_address[7:]
    elif ip_address.startswith("https://"):
        ip_address = ip_address[8:]
    
    test_connection(ip_address)

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n测试已取消")
    except Exception as e:
        print(f"\n测试出错: {e}")
        sys.exit(1) 