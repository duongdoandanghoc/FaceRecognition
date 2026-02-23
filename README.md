# Ứng dụng Nhận diện Khuôn mặt với ESP32-CAM và Python

## Giới thiệu

Dự án này xây dựng một hệ thống nhận diện khuôn mặt thời gian thực, sử dụng thư viện `face_recognition` của Python và được tích hợp với camera ESP32-CAM. Hệ thống có khả năng thu thập hình ảnh, xác định các cá nhân đã được huấn luyện trước và tự động ghi lại sự hiện diện của họ, phù hợp cho các ứng dụng như điểm danh tự động hoặc giám sát an ninh. Ngoài ra, dự án cũng hỗ trợ sử dụng webcam của máy tính như một nguồn video đầu vào thay thế.

## Tính năng

*   **Nhận diện khuôn mặt thời gian thực:** Sử dụng thư viện `face_recognition` để xác định và nhận dạng các khuôn mặt đã biết từ luồng video trực tiếp.
*   **Tích hợp ESP32-CAM:** Thu thập hình ảnh không dây từ ESP32-CAM thông qua giao thức HTTP, cho phép triển khai hệ thống một cách linh hoạt trong nhiều môi trường khác nhau.
*   **Điểm danh tự động:** Tự động ghi lại tên và thời gian của những người được nhận diện vào một tệp CSV (`Điểm Danh.csv`), tạo ra một hệ thống điểm danh đơn giản và hiệu quả.
*   **Hỗ trợ Webcam:** Cung cấp một kịch bản thay thế (`PC_cam_test.py`) để dễ dàng chuyển đổi và sử dụng webcam của máy tính làm nguồn cấp dữ liệu video.
*   **Giao diện trực quan:** Hiển thị luồng video trực tiếp với các hộp giới hạn (bounding box) và nhãn tên của những người được nhận diện, giúp người dùng dễ dàng theo dõi.

## Cấu trúc dự án

```
FaceRecognition/
├── FACEID_ESP32CAM/
│   ├── ESP32Cam.py             # Kịch bản chính để nhận diện khuôn mặt với ESP32-CAM
│   ├── PC_cam_test.py          # Kịch bản thay thế để nhận diện khuôn mặt với webcam PC
│   ├── Images_Basic/           # Thư mục chứa ảnh mẫu để huấn luyện mô hình
│   │   ├── DuongDoan.jpg
│   │   └── ...
│   └── Điểm Danh.csv           # Tệp CSV lưu trữ dữ liệu điểm danh
├── esp32cam/
│   ├── esp32cam.ino            # Mã nguồn Arduino cho ESP32-CAM (cấu hình camera, Wi-Fi, web server)
│   ├── WifiCam.hpp             # Tệp tiêu đề cho cấu hình Wi-Fi
│   ├── handlers.cpp            # Xử lý các yêu cầu HTTP
│   └── images.h                # Dữ liệu hình ảnh cho màn hình OLED (nếu có)
├── KTVXL.fzz                   # Tệp thiết kế mạch Fritzing
├── LICENSE                     # Giấy phép của dự án
├── README.md                   # Tệp tài liệu hướng dẫn này
└── ...                         # Các tệp tài liệu và báo cáo khác
```

## Yêu cầu

### Phần cứng

*   **ESP32-CAM:** Bo mạch phát triển camera.
*   **Máy tính:** Để chạy kịch bản Python xử lý hình ảnh.
*   **Webcam (tùy chọn):** Nếu bạn muốn sử dụng webcam của PC.

### Phần mềm

*   **Python 3.x**
*   **Arduino IDE:** Để lập trình và nạp mã nguồn cho ESP32-CAM.

### Thư viện Python

Cài đặt các thư viện cần thiết bằng trình quản lý gói `pip`:

```bash
pip install opencv-python numpy face_recognition requests
```

## Hướng dẫn cài đặt

1.  **Clone kho lưu trữ:**

    ```bash
    git clone https://github.com/duongdoandanghoc/FaceRecognition.git
    cd FaceRecognition
    ```

2.  **Cấu hình ESP32-CAM:**

    *   Mở tệp `esp32cam/esp32cam.ino` bằng Arduino IDE.
    *   Cập nhật `WIFI_SSID` và `WIFI_PASS` (dòng 14-15) với thông tin đăng nhập mạng Wi-Fi của bạn.
    *   Nạp mã nguồn vào bo mạch ESP32-CAM. Đảm bảo ESP32-CAM và máy tính chạy Python được kết nối vào cùng một mạng Wi-Fi.
    *   Sau khi nạp thành công, mở Serial Monitor trong Arduino IDE để xem địa chỉ IP của ESP32-CAM. Hãy ghi lại địa chỉ IP này.

3.  **Chuẩn bị ảnh mẫu:**

    *   Thêm các hình ảnh của những người bạn muốn nhận diện vào thư mục `FACEID_ESP32CAM/Images_Basic/`.
    *   Đổi tên tệp ảnh thành tên của người tương ứng (ví dụ: `ElonMusk.jpg`, `SteveJobs.png`).

## Cách sử dụng

### 1. Chế độ ESP32-CAM

1.  Mở tệp `FACEID_ESP32CAM/ESP32Cam.py`.
2.  Cập nhật biến `esp32cam_url` (dòng 10) với địa chỉ IP của ESP32-CAM mà bạn đã ghi lại ở bước trên (ví dụ: `http://192.168.1.100/640x480.jpg`).
3.  Chạy kịch bản Python:

    ```bash
    cd FACEID_ESP32CAM
    python ESP32Cam.py
    ```

    Một cửa sổ sẽ hiển thị luồng video từ ESP32-CAM, các khuôn mặt được nhận diện sẽ được đánh dấu và thông tin điểm danh sẽ được ghi lại.

### 2. Chế độ Webcam PC

1.  Chạy kịch bản `PC_cam_test.py`:

    ```bash
    cd FACEID_ESP32CAM
    python PC_cam_test.py
    ```

    Hệ thống sẽ sử dụng webcam của máy tính để thực hiện nhận diện và điểm danh tương tự.

### Kết quả điểm danh

*   Dữ liệu điểm danh được lưu trong tệp `FACEID_ESP32CAM/Điểm Danh.csv`.
*   Mỗi khi một người được nhận diện lần đầu trong một phiên chạy, tên và thời gian sẽ được ghi lại vào tệp này.

## Đóng góp

Mọi đóng góp để cải thiện dự án đều được hoan nghênh. Nếu bạn có ý tưởng, phát hiện lỗi, hoặc muốn thêm tính năng mới, vui lòng tạo một **Issue** hoặc gửi một **Pull Request**.

## Giấy phép

Dự án này được phát hành dưới Giấy phép MIT. Vui lòng xem tệp [LICENSE](LICENSE) để biết thêm chi tiết.

---

*Tài liệu này được tạo bởi **Manus AI** vào ngày 23 tháng 2 năm 2026.*
