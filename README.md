Telemetry Monitoring System
Система для сбора, отображения и анализа телеметрии сотовых сетей (LTE/5G/GSM) в реальном времени.

Возможности
Приём телеметрии через ZeroMQ (порт 5555)

Сохранение в PostgreSQL — все данные пишутся в БД

GUI на ImGui + ImPlot:

Отображение трека на OpenStreetMap

Тепловая карта сигнала (RSRP/RSRQ/RSSI/Altitude) с IDW-интерполяцией

Графики изменения сигнала по времени (для каждого PCI)

Загрузка и просмотр исторических сессий из log.json

Стек технологий
Компонент	Технологии
GUI	SDL2, OpenGL, ImGui, ImPlot
Сеть	ZeroMQ
База данных	PostgreSQL, libpq
Карты	OpenStreetMap (загрузка тайлов через libcurl)
Изображения	stb_image, stb_image_write
Сборка
bash
mkdir build && cd build
cmake ..
make
Зависимости
SDL2, OpenGL, GLEW

libpq (PostgreSQL client)

libzmq

libcurl

ImGui, ImPlot (included)

Запуск
Настроить PostgreSQL и создать таблицу cell_telemetry (см. схему в Database.cpp)

Запустить сервер: ./telemetry_server

Отправить тестовые данные: echo "12:34:56|55.0302|82.9204|150|5|LTE|pci=123|rsrp=-85|rsrq=-12|rssi=-65|sinr=15" | nc localhost 5555

Формат входных данных
text
timestamp|lat|lon|alt|accuracy|type|param1=value1|param2=value2...
Пример для LTE:

text
14:30:25|55.0302|82.9204|150.5|5.0|LTE|pci=301|rsrp=-85|rsrq=-12|rssi=-65|sinr=15
Управление картой
ПКМ + перетаскивание — панорамирование

Двойной клик — приближение в точке

Кнопки +/- — изменение зума

Кнопки центрирования — на GPS позицию или Новосибирск

Структура проекта
Файл	Назначение
NetworkServer.cpp	ZeroMQ сервер
Database.cpp	PostgreSQL сохранение
GuiManager.cpp	Главное окно и цикл рендеринга
SessionRenderer.cpp	Отрисовка карты, графиков, тепловой карты
TileManager.cpp	Загрузка OSM тайлов
Heatmap.cpp	Генерация тепловой карты (IDW)
TelemetryParser.cpp	Парсинг входящих данных
Кэширование
Тайлы OSM кэшируются в tile_cache/

Тепловые карты — в heatmap_cache/
