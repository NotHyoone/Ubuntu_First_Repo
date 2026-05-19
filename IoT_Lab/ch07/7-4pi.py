import io
import cv2
import paho.mqtt.client as mqtt

broker_ip = "localhost"

client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
client.connect(broker_ip, 1883) # 1883 포트로 mosquitto에 접속
client.loop_start() # 메시지 루프를 실행하는 스레드 생성

# 카메라 객체를 생성하고 촬영한 사진 크기를 640x480으로 설정
camera = cv2.VideoCapture(0, cv2.CAP_V4L)
camera.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
camera.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)

# 프레임을 임시 저장할 버퍼 개수를 1로 설정
buffer_size = 1
camera.set(cv2.CAP_PROP_BUFFERSIZE, buffer_size)

while True:
	key = input("사진촬영계속?(종료는 stop 입력)>>")
	if(key == "stop"): # stop이 입력되면 while 문을 벗어남
		break
	# 버퍼에 저장된 모든 프레임을 버리고 새 프레임 읽기
	for i in range(buffer_size+1): 
		ret, frame = camera.read()
	
	im_bytes = cv2.imencode('.jpg', frame)[1].tobytes() # 바이트 배열로 저장
	client.publish("jpeg", im_bytes, qos = 0) # 이미지 전송

camera.release() # 카메라 사용 끝내기
client.loop_stop() # 메시지 루프를 실행하는 스레드 종료
client.disconnect()
