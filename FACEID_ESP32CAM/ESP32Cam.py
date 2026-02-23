import cv2 #Thư viện OpenCV dùng để xử lý hình ảnh và video
import numpy as np #Thư viện hỗ trợ xử lý mảng
import face_recognition #Thư viện nhận diện khuôn mặt
import os #Dùng để thao tác với hệ thống tệp
from datetime import datetime #Lấy thời gian hiện tại
import requests #Thực hiện HTTP request, trong trường hợp này là lấy hình ảnh từ ESP32-CAM
import urllib #Hỗ trợ xử lý URL

#Địa chỉ của ESP32CAM
esp32cam_url = 'http://192.168.11.131/640x480.jpg'


# Hàm lấy hình ảnh từ ESP32CAM
#Gửi HTTP request đến địa chỉ ESP32-CAM và nhận dữ liệu ảnh
#Chuyển đổi dữ liệu từ byte sang mảng NumPy và giải mã ảnh
def get_esp32cam_image():
    try:
        response = requests.get(esp32cam_url, timeout=10)
        if response.status_code == 200:
            img_array = np.array(bytearray(response.content), dtype=np.uint8)
            img = cv2.imdecode(img_array, -1)
            return img
    except Exception as e:
        print(f"Lỗi khi tải hình ảnh từ ESP32-CAM: {str(e)}")
    return None

#Tải dữ liệu hình ảnh mẫu
path = 'Images_Basic'
images = []
classNames = []
mylist = os.listdir(path)
print(mylist)
#Đọc ảnh mẫu và lưu tên lớp
for cl in mylist:
    curImg = cv2.imread(f'{path}/{cl}')
    images.append(curImg)
    classNames.append(os.path.splitext(cl)[0])
print(classNames)

#Tìm mã hóa khuôn mặt từ ảnh mẫu
#Chuyển ảnh từ định dạng BGR sang RGB (yêu cầu của thư viện face_recognition)
#Tạo mã hóa khuôn mặt từ ảnh và thêm vào danh sách
def find_encodings(images):
    encodeList = []
    i = 0
    for img in images:
        img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
        encode = face_recognition.face_encodings(img)[0]
        encodeList.append(encode)
        print(f'Encoding {i}/{len(mylist)} DONE!')
        i=i+1
    return encodeList

#Ghi danh sách điểm danh
#Kiểm tra xem tên đã được điểm danh chưa
#Nếu chưa, ghi thêm tên và thời gian vào tệp Attendance.csv
def markAttendance(name):
    with open('Điểm Danh.csv', 'r+') as f:
        myDataList = f.readlines()
        nameList = []
        for line in myDataList:
            entry = line.split(',')
            nameList.append(entry[0])
        if name not in nameList:
            now = datetime.now()
            dtString = now.strftime('%Y-%m-%d %H:%M:%S')
            f.writelines(f'\n{name},{dtString}')


encodelistknown = find_encodings(images)
print('Encoding Complete!')

#Nhận ảnh từ camera và xử lý trước:
#Thu nhỏ kích thước ảnh để tăng tốc độ xử lý.
#Chuyển đổi sang định dạng RGB.
while True:
    # Chụp ảnh từ ESP32-CAM
    img = get_esp32cam_image()

    if img is not None:
        imgS = cv2.resize(img, (0, 0), None, 0.25, 0.25)
        imgS = cv2.cvtColor(imgS, cv2.COLOR_BGR2RGB)

        #Xác định vị trí và mã hóa khuôn mặt trong khung hình
        faceCurFrame = face_recognition.face_locations(imgS)
        encodesCurFrame = face_recognition.face_encodings(imgS, faceCurFrame)

        #So sánh khuôn mặt nhận diện với dữ liệu mẫu
        #compare_faces: So khớp khuôn mặt hiện tại với danh sách mã hóa
        #face_distance: Tính khoảng cách (độ giống nhau)
        #argmin: Tìm chỉ số khuôn mặt giống nhất
        for encodeFace, faceLoc in zip(encodesCurFrame, faceCurFrame):
            matches = face_recognition.compare_faces(encodelistknown, encodeFace)
            faceDis = face_recognition.face_distance(encodelistknown, encodeFace)
            print(faceDis)
            matchIndex = np.argmin(faceDis)

            #Hiển thị kết quả nhận diện:
            #Vẽ hình chữ nhật quanh khuôn mặt
            #Hiển thị tên của người nhận diện
            if matches[matchIndex]:
                name = classNames[matchIndex].upper()
                print(name)
                y1, x2, y2, x1 = faceLoc
                y1, x2, y2, x1 = y1 * 4, x2 * 4, y2 * 4, x1 * 4
                cv2.rectangle(img, (x1, y1), (x2, y2), (0, 255, 0), 2)
                cv2.rectangle(img, (x1, y2 - 35), (x2, y2), (0, 255, 0), cv2.FILLED)
                cv2.putText(img, name, (x1 + 6, y2 - 6), cv2.FONT_HERSHEY_COMPLEX, 1, (255, 255, 255), 2)
                markAttendance(name)

        #Hiển thị khung hình nhận diện trên màn hình
        cv2.imshow('ESP32-CAM', img)
        cv2.waitKey(1)
