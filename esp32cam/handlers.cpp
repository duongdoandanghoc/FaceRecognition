#include "WifiCam.hpp"
#include <WiFi.h>
#include <StreamString.h>
#include <uri/UriBraces.h>

// Trang HTML chính để hiển thị danh sách độ phân giải và các định dạng ảnh
static const char FRONTPAGE[] = R"EOT(
<!doctype html>
<title>esp32cam WifiCam example</title>
<style>
table,th,td { border: solid 1px #000000; border-collapse: collapse; }
th,td { padding: 0.4rem; }
a { text-decoration: none; }
footer { margin-top: 1rem; }
</style>
<body>
<h1>esp32cam WifiCam example</h1>
<table>
<thead>
<tr><th>BMP<th>JPG<th>MJPEG
<tbody id="resolutions">
<tr><td colspan="3">loading
</table>
<footer>Powered by <a href="https://esp32cam.yoursunny.dev/">esp32cam</a></footer>
<script type="module">
// Script để lấy danh sách độ phân giải và cập nhật vào bảng HTML
async function fetchText(uri, init) {
  const response = await fetch(uri, init);
  if (!response.ok) {
    throw new Error(await response.text());
  }
  return (await response.text()).trim().replaceAll("\r\n", "\n");
}

try {
  const list = (await fetchText("/resolutions.csv")).split("\n");
  document.querySelector("#resolutions").innerHTML = list.map((r) => `<tr>${
    ["bmp", "jpg", "mjpeg"].map((fmt) => `<td><a href="/${r}.${fmt}">${r}</a>`).join("")
  }`).join("");
} catch (err) {
  document.querySelector("#resolutions td").textContent = err.toString();
}
</script>
)EOT";

// Hàm phục vụ ảnh tĩnh (BMP hoặc JPG)
static void
serveStill(bool wantBmp)
{
  auto frame = esp32cam::capture(); // Chụp ảnh từ camera
  if (frame == nullptr) {
    Serial.println("capture() failure");
    server.send(500, "text/plain", "still capture error\n");
    return;
  }
  Serial.printf("capture() success: %dx%d %zub\n", frame->getWidth(), frame->getHeight(),
                frame->size());

  // Nếu muốn BMP, chuyển đổi từ JPG sang BMP
  if (wantBmp) {
    if (!frame->toBmp()) {
      Serial.println("toBmp() failure");
      server.send(500, "text/plain", "convert to BMP error\n");
      return;
    }
    Serial.printf("toBmp() success: %dx%d %zub\n", frame->getWidth(), frame->getHeight(),
                  frame->size());
  }

  // Gửi dữ liệu ảnh đến máy khách
  server.setContentLength(frame->size());
  server.send(200, wantBmp ? "image/bmp" : "image/jpeg");
  WiFiClient client = server.client();
  frame->writeTo(client);
}

// Hàm phục vụ video MJPEG
static void
serveMjpeg()
{
  Serial.println("MJPEG streaming begin");
  WiFiClient client = server.client();
  auto startTime = millis();
  int nFrames = esp32cam::Camera.streamMjpeg(client); // Stream MJPEG
  auto duration = millis() - startTime;
  Serial.printf("MJPEG streaming end: %dfrm %0.2ffps\n", nFrames, 1000.0 * nFrames / duration);
}

// Thêm các route xử lý HTTP cho máy chủ
void
addRequestHandlers()
{
  // Route trang chính
  server.on("/", HTTP_GET, [] {
    server.setContentLength(sizeof(FRONTPAGE));
    server.send(200, "text/html");
    server.sendContent(FRONTPAGE, sizeof(FRONTPAGE));
  });

  // Route cho robots.txt
  server.on("/robots.txt", HTTP_GET,
            [] { server.send(200, "text/html", "User-Agent: *\nDisallow: /\n"); });

  // Route để lấy danh sách độ phân giải dưới dạng CSV
  server.on("/resolutions.csv", HTTP_GET, [] {
    StreamString b;
    for (const auto& r : esp32cam::Camera.listResolutions()) {
      b.println(r);
    }
    server.send(200, "text/csv", b);
  });

  // Route để xử lý các định dạng ảnh dựa trên URL
  server.on(UriBraces("/{}x{}.{}"), HTTP_GET, [] {
    long width = server.pathArg(0).toInt(); // Lấy chiều rộng từ URL
    long height = server.pathArg(1).toInt(); // Lấy chiều cao từ URL
    String format = server.pathArg(2); // Lấy định dạng từ URL
    if (width == 0 || height == 0 || !(format == "bmp" || format == "jpg" || format == "mjpeg")) {
      server.send(404); // Trả về 404 nếu thông số không hợp lệ
      return;
    }

    // Kiểm tra độ phân giải có hợp lệ không
    auto r = esp32cam::Camera.listResolutions().find(width, height);
    if (!r.isValid()) {
      server.send(404, "text/plain", "non-existent resolution\n");
      return;
    }
    if (r.getWidth() != width || r.getHeight() != height) {
      server.sendHeader("Location",
                        String("/") + r.getWidth() + "x" + r.getHeight() + "." + format);
      server.send(302); // Chuyển hướng đến độ phân giải hợp lệ gần nhất
      return;
    }

    // Thay đổi độ phân giải camera
    if (!esp32cam::Camera.changeResolution(r)) {
      Serial.printf("changeResolution(%ld,%ld) failure\n", width, height);
      server.send(500, "text/plain", "changeResolution error\n");
    }
    Serial.printf("changeResolution(%ld,%ld) success\n", width, height);

    // Phục vụ nội dung dựa trên định dạng
    if (format == "bmp") {
      serveStill(true);
    } else if (format == "jpg") {
      serveStill(false);
    } else if (format == "mjpeg") {
      serveMjpeg();
    }
  });
}
