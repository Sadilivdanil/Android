// Main.kt
import kotlin.concurrent.thread
import kotlin.random.Random

fun main() {
    // Первая симуляция
    println("=== ПЕРВОЕ ЗАДАНИЕ: СИМУЛЯЦИЯ ДВИЖЕНИЯ ===")

    val humans = mutableListOf<Human>()
    val names = listOf("Иванов", "Петров", "Сидоров", "Кузнецов", "Смирнов", "Попов", "Васильев")

    repeat(18) { index ->
        val randomName = "${names.random()} ${names.random()}"
        val randomAge = Random.nextInt(18, 65)
        val randomSpeed = Random.nextDouble(1.0, 3.0)
        humans.add(Human("Человек ${index + 1}: $randomName", randomAge, randomSpeed))
    }

    val simulationTime = 10
    println("Участников: ${humans.size}, Время: ${simulationTime}сек")
    println("=".repeat(50))

    for (second in 1..simulationTime) {
        println("\n--- Секунда $second ---")
        humans.forEach { it.moveWithLog(1.0) }
        Thread.sleep(500)
    }

    println("\n=== РЕЗУЛЬТАТЫ ПЕРВОГО ЗАДАНИЯ ===")
    humans.forEachIndexed { index, human ->
        val pos = human.getPosition()
        println("${index + 1}. ${human.getFullName()} - (${"%.2f".format(pos.first)}, ${"%.2f".format(pos.second)})")
    }

    println("\n" + "=".repeat(60) + "\n")

    // Вторая симуляция
    println("=== ВТОРОЕ ЗАДАНИЕ: НАСЛЕДОВАНИЕ И ПОТОКИ ===")

    val human1 = Human("Иван", 25, 1.0)
    val human2 = Human("Петр", 30, 1.5)
    val human3 = Human("Анна", 28, 2.0)
    val driver = Driver("Алексей", 3.0)

    val entities = listOf(human1, human2, human3, driver)

    println("Запуск параллельного движения...")
    val threads = entities.map { entity ->
        thread {
            repeat(5) {
                when (entity) {
                    is Human -> entity.moveWithLog()
                    is Driver -> entity.moveWithLog()
                }
                Thread.sleep(400)
            }
        }
    }

    threads.forEach { it.join() }

    println("\n=== ФИНАЛЬНЫЕ ПОЗИЦИИ ===")
    entities.forEach {
        val type = if (it is Driver) "Водитель" else "Человек"
        val name = if (it is Human) it.getFullName() else (it as Driver).name
        val pos = it.getPosition()
        println("$type $name: (${"%.1f".format(pos.first)}, ${"%.1f".format(pos.second)})")
    }
}