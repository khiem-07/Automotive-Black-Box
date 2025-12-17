Automotive Black Box(STM32)
Hệ thống Hộp đen và Cảnh báo khẩn cấp cho ô tô dựa trên STM32 & FreeRTOS

Dự án này là một hệ thống nhúng thời gian thực mô phỏng chức năng của "Hộp đen" trên ô tô. Hệ thống có khả năng giám sát thông số vận hành (Tốc độ, RPM, Nhiệt độ), ghi dữ liệu hành trình vào thẻ nhớ, tự động phát hiện tai nạn dựa trên gia tốc và thực hiện cuộc gọi khẩn cấp (eCall) kèm tọa độ GPS.

Tính năng chính
Giám sát thời gian thực: Thu thập dữ liệu Tốc độ (Speed), Vòng tua máy (RPM) và Nhiệt độ động cơ qua giao thức CAN Bus (Mô phỏng chuẩn OBD-II).

Ghi dữ liệu (Black Box): Lưu trữ lịch sử vận hành vào thẻ nhớ SD (định dạng CSV) với mốc thời gian thực từ DS3231. Dữ liệu này dùng để điều tra sau va chạm.

Phát hiện tai nạn (Crash Detection): Thuật toán thông minh phát hiện sự giảm tốc đột ngột (ví dụ: từ 100km/h về 0km/h trong tích tắc) để xác định va chạm.

Cảnh báo khẩn cấp (eCall): Tự động gửi tin nhắn SMS chứa tọa độ và thực hiện cuộc gọi đến số cứu hộ thông qua module SIM khi phát hiện tai nạn.

Giao diện HMI: Hiển thị thông số trực quan trên màn hình màu TFT ST7735.

Hệ điều hành: Sử dụng FreeRTOS để quản lý đa nhiệm (Thu thập dữ liệu, Hiển thị, Ghi thẻ nhớ) đảm bảo độ trễ thấp nhất.
