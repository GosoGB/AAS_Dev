import os
import json
import sqlite3
from datetime import datetime
from collections import defaultdict


def setup_database(db_path):
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS todos (
            id INTEGER PRIMARY KEY,
            path TEXT,
            count INTEGER
        )
    ''')
    conn.commit()
    return conn


def insert_todo_count(conn, todo_info):
    current_datetime = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    cursor = conn.cursor()
    cursor.execute('INSERT INTO todos (datetime, todo_count) VALUES (?, ?)', (current_datetime, todo_info))
    conn.commit()


def count_todo_by_file(directory):
    total_todo_count = 0
    file_todo_counts = {}

    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith('.cpp'):
                file_path = os.path.join(root, file)
                todo_count = 0

                with open(file_path, 'r', encoding='utf-8') as f:
                    for line in f:
                        if '* @todo' in line:
                            todo_count += 1
                
                if todo_count > 0:
                    relative_path = os.path.relpath(file_path, start=directory)
                    file_todo_counts[relative_path] = todo_count
                    total_todo_count += todo_count

    return total_todo_count, file_todo_counts


def to_structured_data(paths, counts):
    tree = {}

    # 문자열 리스트를 기반으로 트리 구조 만들기
    for path in paths:
        index = paths.index(path)
        parts = path.split('\\')
        current_level = tree

        for part in parts:
            if part not in current_level:
                if str(part).endswith(".cpp"):
                    current_level[part] = counts[index]
                else:
                    current_level[part] = {}
            current_level = current_level[part]

    return tree



def create_todo_count_info(data):
    result = {}
    
    # Iterate through the items in the original data
    for key, value in data.items():
        print(f'key: {key},  value: {value} in data.items()')
        total = 0
        files = {}
        subdirs = []

        if isinstance(value, dict):
            for subkey, subvalue in value.items():
                print(f'subkey: {subkey},  subvalue: {subvalue} in value.items()')
                if isinstance(subvalue, dict):
                    print('if isinstance(subvalue, dict):')
                    files[subkey] = create_todo_count_info(value)
                    # subdir = create_todo_count_info(subvalue)  # Recursive call
                    # subdirs.append({subkey: subdir})
                    # total += subdir['total']  # Add the total of the subdir
                else:
                    # If it's a file, add it to files
                    files[subkey] = subvalue
                    total += subvalue  # Add file value to total
        
        result[key] = {
            'total': total,
            'files': files,
            'subdir': subdirs
        }

    # 'total' 키를 명확히 설정
    total_count = sum(value['total'] for value in result.values() if isinstance(value, dict))
    return {**result, 'total': total_count}  # 최상위에 total 추가


current_directory = os.path.dirname(os.path.abspath(__file__))
project_directory = os.path.dirname(os.path.dirname(current_directory))
src_code_directory = os.path.join(project_directory, 'MUFFIN', 'lib', 'MUFFIN', 'src')  # 상대 경로 지정

total_count, todos_per_file = count_todo_by_file(src_code_directory)
paths  = [paths  for paths, counts in todos_per_file.items()]
counts = [counts for paths, counts in todos_per_file.items()]

structured_data = to_structured_data(paths, counts)


def transform2(data):
    result = {}
    total = 0
    files = {}
    subdirs = []

    for datum in data.items():
        key, value = datum
        if isinstance(value, dict):
            print(f'value.items(): {value.items()}')
            value_items = value.items()
            for item in value_items:
                sub_key, sub_value = item
                if sub_key.endswith('.cpp'):
                    files[sub_key] = sub_value
                    result[key] = { "files": files }
                else:
                    

        # if isinstance(value, dict):
        #     print(f'value: {value}')
        #     for key, value in data:
        #         print(f'key: {key}  value: {value}')
        #         files[key] = value
        # else:
        #     print(f'data: {data}')
    return result


def transform(data):
    result = {}
    total = 0
    files = {}
    subdirs = []

    for key, value in data.items():
        if str(key).endswith('.cpp') == True:
            files[key] = value
        elif isinstance(value, dict):
            subdirs.append(transform2(value))
        break
    
    result["total"] = total
    result["files"] = files
    if len(subdirs) != 0:
        result["subdirs"] = subdirs
    return result



print(json.dumps(transform2(structured_data)))
# todo_count_info = create_todo_count_info(structured_data)
# json_output = json.dumps(todo_count_info, indent=3)
# print(json_output)




"""
print(f'------------------------------------')
print(f' Total @todo counts: {total_todos}  ')
print(f'------------------------------------')

# 각 폴더별 TODO 카운트 출력
print_todo_counts(todos_per_folder, folder_structure)

# SQLite 데이터베이스에 todo 카운트 저장
db_directory = os.path.join(current_directory, 'sqlite3.db')  # 상대 경로 지정
conn = setup_database(db_directory)
for folder, count in todos_per_folder.items():
    insert_todo_count(conn, folder, count)

for file_path, count in todos_per_file.items():
    insert_todo_count(conn, file_path, count)

conn.close()  # 데이터베이스 연결 종료
"""