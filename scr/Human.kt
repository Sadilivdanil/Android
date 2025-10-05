// Human.kt
import kotlin.math.PI
import kotlin.math.cos
import kotlin.math.sin
import kotlin.random.Random
//класс человек
class Human(
    private val fullName: String,
    private val age: Int,
    override val speed: Double
) : Movable {
    override var x: Double = 0.0
    override var y: Double = 0.0
// метод перемещения
    override fun move(timeStep: Double) {
        val randomAngle = Random.nextDouble(0.0, 2 * PI)
        val dx = speed * timeStep * cos(randomAngle)
        val dy = speed * timeStep * sin(randomAngle)
        x += dx
        y += dy
    }
//метод для получения имени геттер
    fun getFullName(): String = fullName
    fun getAge(): Int = age
    fun getCurrentSpeed(): Double = speed
//метод для перемешения с выводом в консоль
    fun moveWithLog(timeStep: Double = 1.0) {
        move(timeStep)
        println("$fullName переместился в позицию (${"%.2f".format(x)}, ${"%.2f".format(y)})")
    }

}
