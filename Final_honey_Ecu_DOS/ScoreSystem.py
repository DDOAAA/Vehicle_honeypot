#스코어링 시스템

import pandas as pd
from scapy.all import rdpcap, IP, TCP, UDP

# 설정
KNOWN_SERVICE_IDS = {0x1234, 0x5678}  # 예시: 허용된 Service ID
NORMAL_PORTS = {30490, 30500, 30491}  # 일반적으로 사용되는 SOME/IP 포트
OEM_SIGNATURES = [b'\x12\x34\x56\x78', b'\x87\x65\x43\x21']  # 예시 시그니처

def score_attacker(packets):
    attacker_data = {}
    
    for pkt in packets:
        if IP in pkt:
            ip = pkt[IP].src
            port = pkt[TCP].dport if TCP in pkt else (pkt[UDP].dport if UDP in pkt else None)
            payload = bytes(pkt[IP].payload)
            
            if ip not in attacker_data:
                attacker_data[ip] = {
                    "count": 0,
                    "bad_ports": 0,
                    "bad_service_id": 0,
                    "oem_match": 0
                }
            
            attacker_data[ip]["count"] += 1
            
            if port and port not in NORMAL_PORTS:
                attacker_data[ip]["bad_ports"] += 1
            
            if any(sig in payload for sig in OEM_SIGNATURES):
                attacker_data[ip]["oem_match"] += 1
            else:
                attacker_data[ip]["bad_service_id"] += 1
    
    results = {}
    for ip, data in attacker_data.items():
        score = 0
        score += 10 if data["count"] > 1000 else int((data["count"] / 1000) * 10)
        score += 10 if data["oem_match"] > 0 else 0
        score += 10 if ip.startswith("10.") or ip.startswith("192.") or ip.startswith("172.") else 0
        score += 5 if data["bad_ports"] > 0 else 0
        score += 5 if data["bad_service_id"] > 0 else 0
        
        results[ip] = {
            "총점": score,
            "패킷 수": data["count"],
            "비정상 포트": data["bad_ports"],
            "허용되지 않은 Service ID": data["bad_service_id"]
        }

    return pd.DataFrame(results).T

# 사용 예시
pcap_path = "/home/doa/vehicle_honeypot/Final_honey_Ecu_DOS/Pcapfiles/Vsomeip_DOS.pcapng"  # pcap 파일 경로
packets = rdpcap(pcap_path)
result_df = score_attacker(packets)
print(result_df)
# result_df.to_csv("attack_score_result.csv")