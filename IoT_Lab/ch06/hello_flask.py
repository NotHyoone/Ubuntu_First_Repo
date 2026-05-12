from flask import Flask

app = Flask(__name__) # Flask 객체 생성

@app.route('/')
def home():
	return '<h2>Hello, Flask</h2>'

if __name__ == '__main__': # 이 프로그램이 독립적으로 실행되는 경우
	app.run(host='0.0.0.0', port=8080, debug=True) # app.run() 함수 실행
