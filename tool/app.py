import os
import sqlite3
from collections import defaultdict
from flask import Flask, render_template



app = Flask(__name__)


def get_todo_counts(db_name='sqlite3.db'):
    app_directory = os.path.dirname(os.path.abspath(__file__))
    db_directory = os.path.join(app_directory, 'sqlite3.db')
    conn = sqlite3.connect(db_directory)
    cursor = conn.cursor()
    
    cursor.execute("SELECT path, count FROM todos")
    rows = cursor.fetchall()

    todo_counts = defaultdict(lambda: {'count': 0, 'files': {}})

    for path, count in rows:
        parts = path.split('\\')  # Windows 경로 구분자 사용
        current = todo_counts

        # 계층 구조 생성
        for part in parts[:-1]:  # 파일 이름 제외
            if part not in current:
                current[part] = {'count': 0, 'files': {}}
            current[part]['count'] += count
            # files 키 확인 후 하위 폴더로 이동
            if 'files' not in current[part]:
                current[part]['files'] = {}
            current = current[part]['files']  # 하위 폴더로 이동

        # 파일 카운트 추가
        filename = parts[-1]
        if filename not in current:
            current[filename] = {'count': 0}  # 파일 초기화
        current[filename]['count'] += count  # 파일 카운트 업데이트

    conn.close()
    return todo_counts


@app.route('/')
def index():
    todo_counts = get_todo_counts()
    return render_template('index.html', todo_counts=todo_counts)

if __name__ == '__main__':
    app.run(debug=True)
