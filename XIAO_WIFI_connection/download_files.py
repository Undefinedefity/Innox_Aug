#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
XIAO ESP32S3 文件下载器
用于从 ESP32S3 开发板的 microSD 卡中下载文件
"""

import requests
import json
import os
import sys
from urllib.parse import urljoin, quote

class ESP32FileDownloader:
    def __init__(self, base_url="http://192.168.4.1"):
        """
        初始化下载器
        
        Args:
            base_url (str): ESP32S3 的 IP 地址，默认为 http://192.168.4.1
        """
        self.base_url = base_url.rstrip('/')
        self.session = requests.Session()
        self.session.timeout = 60  # 增加超时时间
        
        # 优化连接设置以提高下载速度
        adapter = requests.adapters.HTTPAdapter(
            pool_connections=10,
            pool_maxsize=10,
            max_retries=3
        )
        self.session.mount('http://', adapter)
        self.session.mount('https://', adapter)
        
    def get_file_list(self):
        """
        获取文件列表
        
        Returns:
            list: 文件信息列表，每个元素包含 name 和 size
        """
        try:
            url = urljoin(self.base_url, "/list")
            print(f"正在获取文件列表: {url}")
            
            response = self.session.get(url)
            response.raise_for_status()
            
            data = response.json()
            return data.get('files', [])
            
        except requests.exceptions.RequestException as e:
            print(f"获取文件列表失败: {e}")
            return []
        except json.JSONDecodeError as e:
            print(f"解析文件列表失败: {e}")
            return []
    
    def download_file(self, filename, save_dir="downloads"):
        """
        下载单个文件
        
        Args:
            filename (str): 要下载的文件名
            save_dir (str): 保存目录
            
        Returns:
            bool: 下载是否成功
        """
        try:
            # 创建保存目录
            os.makedirs(save_dir, exist_ok=True)
            
            # 构建下载 URL
            download_url = urljoin(self.base_url, f"/file?name={quote(filename)}")
            save_path = os.path.join(save_dir, filename)
            
            print(f"正在下载: {filename}")
            print(f"下载地址: {download_url}")
            print(f"保存路径: {save_path}")
            
            # 下载文件
            response = self.session.get(download_url, stream=True)
            response.raise_for_status()
            
            # 获取文件大小
            content_length = response.headers.get('content-length', '0')
            try:
                total_size = int(content_length)
            except ValueError:
                print(f"警告: 无法解析文件大小 '{content_length}'，将显示为未知大小")
                total_size = 0
            
            # 保存文件
            with open(save_path, 'wb') as f:
                downloaded = 0
                # 增加缓冲区大小以提高下载速度
                for chunk in response.iter_content(chunk_size=32768):  # 32KB 缓冲区
                    if chunk:
                        f.write(chunk)
                        downloaded += len(chunk)
                        
                        # 显示下载进度
                        if total_size > 0:
                            progress = (downloaded / total_size) * 100
                            print(f"\r下载进度: {progress:.1f}% ({downloaded}/{total_size} 字节)", end='', flush=True)
                        else:
                            print(f"\r已下载: {downloaded} 字节", end='', flush=True)
            
            print(f"\n✅ 文件下载完成: {filename}")
            return True
            
        except requests.exceptions.RequestException as e:
            print(f"\n❌ 下载失败: {filename} - {e}")
            return False
        except Exception as e:
            print(f"\n❌ 保存文件失败: {filename} - {e}")
            return False
    
    def download_all_files(self, save_dir="downloads"):
        """
        下载所有文件
        
        Args:
            save_dir (str): 保存目录
            
        Returns:
            tuple: (成功数量, 总数量)
        """
        files = self.get_file_list()
        
        if not files:
            print("没有找到可下载的文件")
            return 0, 0
        
        print(f"找到 {len(files)} 个文件:")
        for i, file_info in enumerate(files, 1):
            print(f"  {i}. {file_info['name']} ({file_info['size']} 字节)")
        
        print(f"\n开始下载到目录: {save_dir}")
        
        success_count = 0
        for file_info in files:
            if self.download_file(file_info['name'], save_dir):
                success_count += 1
        
        return success_count, len(files)
    
    def interactive_download(self, save_dir="downloads"):
        """
        交互式下载 - 让用户选择要下载的文件
        
        Args:
            save_dir (str): 保存目录
        """
        files = self.get_file_list()
        
        if not files:
            print("没有找到可下载的文件")
            return
        
        print(f"找到 {len(files)} 个文件:")
        for i, file_info in enumerate(files, 1):
            print(f"  {i}. {file_info['name']} ({file_info['size']} 字节)")
        
        print("\n选择下载方式:")
        print("1. 下载所有文件")
        print("2. 选择特定文件")
        print("3. 退出")
        
        while True:
            try:
                choice = input("\n请输入选择 (1-3): ").strip()
                
                if choice == "1":
                    success, total = self.download_all_files(save_dir)
                    print(f"\n下载完成: {success}/{total} 个文件成功")
                    break
                    
                elif choice == "2":
                    while True:
                        try:
                            file_num = input("请输入文件编号 (多个文件用逗号分隔): ").strip()
                            file_indices = [int(x.strip()) - 1 for x in file_num.split(',')]
                            
                            success_count = 0
                            for idx in file_indices:
                                if 0 <= idx < len(files):
                                    if self.download_file(files[idx]['name'], save_dir):
                                        success_count += 1
                                else:
                                    print(f"无效的文件编号: {idx + 1}")
                            
                            print(f"\n下载完成: {success_count}/{len(file_indices)} 个文件成功")
                            break
                            
                        except ValueError:
                            print("请输入有效的数字")
                        except KeyboardInterrupt:
                            print("\n下载已取消")
                            break
                    break
                    
                elif choice == "3":
                    print("退出下载器")
                    break
                    
                else:
                    print("请输入有效的选择 (1-3)")
                    
            except KeyboardInterrupt:
                print("\n操作已取消")
                break

def main():
    """主函数"""
    print("=" * 50)
    print("📁 XIAO ESP32S3 文件下载器")
    print("=" * 50)
    
    # 获取 ESP32S3 的 IP 地址（AP模式默认IP）
    default_ip = "192.168.4.1"
    ip_input = input(f"请输入 ESP32S3 的 IP 地址 (AP模式默认: {default_ip}): ").strip()
    
    if not ip_input:
        ip_input = default_ip
    
    # 确保 IP 地址格式正确
    if not ip_input.startswith("http"):
        ip_input = f"http://{ip_input}"
    
    # 创建下载器
    downloader = ESP32FileDownloader(ip_input)
    
    # 测试连接
    print(f"\n正在测试连接到: {ip_input}")
    try:
        files = downloader.get_file_list()
        if files:
            print(f"✅ 连接成功！找到 {len(files)} 个文件")
        else:
            print("⚠️  连接成功，但没有找到文件")
    except Exception as e:
        print(f"❌ 连接失败: {e}")
        print("请检查:")
        print("1. ESP32S3 是否已启动AP模式")
        print("2. 是否已连接到ESP32的WiFi热点 (XIAO_ESP32S3_FileServer)")
        print("3. IP 地址是否正确 (默认: 192.168.4.1)")
        print("4. 网络连接是否正常")
        return
    
    # 开始交互式下载
    downloader.interactive_download()

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n\n程序已退出")
    except Exception as e:
        print(f"\n程序出错: {e}")
        sys.exit(1) 