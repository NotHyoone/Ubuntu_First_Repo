from flask import Flask

app = Flask(__name__) # Flask 객체 생성

balance = 0 # 잔액을 저장하는 변수
@app.route('/<op>/<int:money>/') # 2개의 variable: op와 money
def bank(op, money):
    global balance # 전역변수 활용
    if op == 'deposit': # 입금
        balance += money
        return "<h2>입금 %d원, 잔액 %d</h2>" % (money, balance)
    elif op == 'withdraw': # 출금
        withdraw = balance if money > balance else money # 출금액은 잔액보다 클 수 없다.
        balance -= withdraw
        return "<h2>출금 %d원, 잔액 %d</h2>" % (withdraw, balance)
    else:
        return "<h2>잘못된 요청입니다.</h2>" # 잘못된 요청
    
@app.route('/inquiry/') # 잔액 조회
def inquiry():
    return "<h2>잔액 %d</h2>" % balance

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8080, debug=True)