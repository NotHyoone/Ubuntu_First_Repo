import paho.mqtt.client as mqtt
import time
import random

flag = "stop" # 온도 전송을 하지 않음을 뜻함
def on_connect(client, userdata, flag, rc, prop=None):
    client.subscribe("command") # command 토픽으로 메시지 구독 신청
    
def on_message(client, userdata, msg):
    global flag # 전역변수 flag 사용
    command = msg.payload.decode('utf-8')
    if command == "start":
        flag = "start" # 온도 전송 시작
        print("온도 전송...")
    elif command == "stop":
        flag = "stop" # 온도 전송 중지
        print("\n전송 중지...")
    else:
        print("명령 오류: ", command)
        
broker_ip = "localhost" # 이 컴퓨터의 브로커에 접속
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
client.on_connect = on_connect 
client.on_message = on_message
client.connect(broker_ip, 1883) # 1883 포트로 브로커에 접속
client.loop_start() # 메시지 루프를 실행하는 스레드 생성

print("시작 명령 대기 ...")
while True:
    if flag == "start":
        temperature = random.randint(0, 40) # 0에서 39사이의 랜덤 정수
        client.publish("room/temp", temperature) # 온도 전송
        print(temperature, end=" ", flush=True) # 온도 출력
    time.sleep(2) # 2초 대기
    
client.loop_stop() # 메시지 루프를 실행하는 스레드 종료
client.disconnect()
print("프로그램 종료")