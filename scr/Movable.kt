interface Movable {
    var x: Double
    var y: Double
    val speed: Double
  // Основной метод перемещения объекта
    fun move(timeStep: Double)  // Убрано значение по умолчанию
// Имеет реализацию по умолчанию - можно переопределить при необходимости
    fun getPosition(): Pair<Double, Double> = Pair(x, y)
}
