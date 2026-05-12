from flask import Flask

app = Flask(__name__) # Flask 객체 생성

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8080, debug=True)