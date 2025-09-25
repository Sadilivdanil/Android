import kotlin.concurrent.thread
import kotlin.random.Random
import kotlin.math.*

fun main() {
    println("=== ВТОРОЕ ЗАДАНИЕ: НАСЛЕДОВАНИЕ И ПОТОКИ ===")

    // Базовый класс Human
    open class Human(val name: String, val speed: Double) {
        var x: Double = 0.0
        var y: Double = 0.0

        open fun move() {
            val angle = Random.nextDouble(0.0, 2 * PI)
            x += speed * cos(angle)
            y += speed * sin(angle)
            println("$name: (${"%.1f".format(x)}, ${"%.1f".format(y)})")
        }
    }

    // Класс Driver наследует от Human
    class Driver(name: String, speed: Double) : Human(name, speed) {
        override fun move() {
            x += speed // Движение только по X (прямолинейное)
            println("ВОДИТЕЛЬ $name: (${"%.1f".format(x)}, ${"%.1f".format(y)})")
        }
    }

    // Создаем объекты: 3 человека и 1 водитель
    val human1 = Human("Иван", 1.0)
    val human2 = Human("Петр", 1.5)
    val human3 = Human("Анна", 2.0)
    val driver = Driver("Алексей", 3.0)

    val entities = listOf(human1, human2, human3, driver)

    // Запускаем каждого в отдельном потоке
    println("Запуск параллельного движения...")
    val threads = entities.map { entity ->
        thread {
            repeat(5) { // Каждый делает 5 движений
                entity.move()
                Thread.sleep(400)
            }
        }
    }

    // Ждем завершения всех потоков
    threads.forEach { it.join() }

    println("\n=== ФИНАЛЬНЫЕ ПОЗИЦИИ ===")
    entities.forEach {
        val type = if (it is Driver) "Водитель" else "Человек"
        println("$type ${it.name}: (${"%.1f".format(it.x)}, ${"%.1f".format(it.y)})")
    }
}
