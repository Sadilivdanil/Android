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

    fun moveWithLog(timeStep: Double = 1.0) {
        move(timeStep)
        println("ВОДИТЕЛЬ $name: (${"%.1f".format(x)}, ${"%.1f".format(y)})")
    }
}
