interface Movable {
    var x: Double
    var y: Double
    val speed: Double

    fun move(timeStep: Double)  // Убрано значение по умолчанию

    fun getPosition(): Pair<Double, Double> = Pair(x, y)
}