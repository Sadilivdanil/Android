// Driver.kt
class Driver(
    val name: String,
    override val speed: Double
) : Movable {
    override var x: Double = 0.0
    override var y: Double = 0.0

    override fun move(timeStep: Double) {  // Убрано значение по умолчанию
        x += speed * timeStep // Движение только по X (прямолинейное)
    }
    // Дополнительный метод для перемещения с логированием
    // timeStep - шаг времени (имеет значение по умолчанию 1.0)
    fun moveWithLog(timeStep: Double = 1.0) {
        move(timeStep)
        println("ВОДИТЕЛЬ $name: (${"%.1f".format(x)}, ${"%.1f".format(y)})")
    }
}

