import kotlin.random.Random
import kotlin.math.*

fun main() {
    // Класс Human для первого задания
    class Human(
        private var fullName: String,
        private var age: Int,
        private var currentSpeed: Double
    ) {
        private var x: Double = 0.0
        private var y: Double = 0.0

        fun getFullName(): String = fullName
        fun getAge(): Int = age
        fun getCurrentSpeed(): Double = currentSpeed
        fun getPosition(): Pair<Double, Double> = Pair(x, y)

        fun move(timeStep: Double = 1.0) {
            val randomAngle = Random.nextDouble(0.0, 2 * PI)
            val dx = currentSpeed * timeStep * cos(randomAngle)
            val dy = currentSpeed * timeStep * sin(randomAngle)
            x += dx
            y += dy
            println("$fullName переместился в позицию (${"%.2f".format(x)}, ${"%.2f".format(y)})")
        }
    }

    println("=== ПЕРВОЕ ЗАДАНИЕ: СИМУЛЯЦИЯ ДВИЖЕНИЯ ===")

    // Создаем 18 человек (по номеру в списке)
    val humans = mutableListOf<Human>()
    val names = listOf("Иванов", "Петров", "Сидоров", "Кузнецов", "Смирнов", "Попов", "Васильев")

    repeat(18) { index ->
        val randomName = "${names.random()} ${names.random()}"
        val randomAge = Random.nextInt(18, 65)
        val randomSpeed = Random.nextDouble(1.0, 3.0)
        humans.add(Human("Человек ${index + 1}: $randomName", randomAge, randomSpeed))
    }

    // Симуляция на 10 секунд
    val simulationTime = 10
    println("Участников: ${humans.size}, Время: ${simulationTime}сек")
    println("=".repeat(50))

    for (second in 1..simulationTime) {
        println("\n--- Секунда $second ---")
        humans.forEach { it.move(1.0) }
        Thread.sleep(500)
    }

    println("\n=== РЕЗУЛЬТАТЫ ПЕРВОГО ЗАДАНИЯ ===")
    humans.forEachIndexed { index, human ->
        val pos = human.getPosition()
        println("${index + 1}. ${human.getFullName()} - (${"%.2f".format(pos.first)}, ${"%.2f".format(pos.second)})")
    }
}