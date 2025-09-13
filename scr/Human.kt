import kotlin.random.Random
import kotlin.math.*

data class Position(val x: Double, val y: Double)

class Human(
    private var fullName: String,
    private var age: Int,
    private var currentSpeed: Double
) {
    private var position: Position = Position(0.0, 0.0)

    // получение значений (геттеры)
    fun getFullName(): String = fullName
    fun getAge(): Int = age
    fun getCurrentSpeed(): Double = currentSpeed
    fun getPosition(): Position = position

    // установка значений (сеттеры)
    fun setFullName(newFullName: String) {
        fullName = newFullName
    }

    fun setAge(newAge: Int) {
        require(newAge >= 0) { "Возраст не может быть отрицательным" }
        age = newAge
    }

    fun setCurrentSpeed(newSpeed: Double) {
        require(newSpeed >= 0) { "Скорость не может быть отрицательной" }
        currentSpeed = newSpeed
    }

    /**
     * Метод движения по модели Random Walk
     * @param timeStep шаг времени в секундах
     */
    fun move(timeStep: Double = 1.0) {
        // Случайный угол от 0 до 360
        val randomAngle = Random.nextDouble(0.0, 2 * PI)

        // Вычисление перемещения по осям x и y
        //dx=скорость x время x косинус угла
        //dy=скорость x время x синус угла
        val dx = currentSpeed * timeStep * cos(randomAngle)
        val dy = currentSpeed * timeStep * sin(randomAngle)

        // Обновление позиции
        position = Position(position.x + dx, position.y + dy)

        println("$fullName переместился в позицию (${"%.2f".format(position.x)}, ${"%.2f".format(position.y)})")
    }

    override fun toString(): String {
        return "Human(fullName='$fullName', age=$age, speed=${"%.1f".format(currentSpeed)} м/с, " +
                "position=(${"%.2f".format(position.x)}, ${"%.2f".format(position.y)}))"
    }
}