import kotlin.random.Random
import kotlin.math.*

// Вспомогательная функция для повторения строки
operator fun String.times(n: Int): String {
    return repeat(n)
}

fun main() {
    // Создаем массив из 18 экземпляров класса Human
    val humans = mutableListOf<Human>()

    // Список возможных имен для генерации
    val names = listOf(
        "Иванов", "Петров", "Сидоров", "Кузнецов", "Смирнов", "Попов", "Васильев", "Павлов",
        "Семенов", "Голубев", "Виноградов", "Богданов", "Воробьев", "Федоров", "Михайлов",
        "Беляев", "Тарасов", "Белов", "Комаров", "Орлов"
    )

    // Создаем 18 так как мой номер в списке 18 человек со случайными параметрами
    repeat(18) { index ->
        val randomName = names.random() + " " + names.random() + " " + names.random()
        val randomAge = Random.nextInt(18, 65)
        val randomSpeed = Random.nextDouble(0.5, 2.5)

        humans.add(Human("Человек ${index + 1}: $randomName", randomAge, randomSpeed))
    }

    // Время симуляции (18 секунд)
    val simulationTime = 18
    val timeStep = 1.0 // шаг времени 1 секунда

    println("=== НАЧАЛО СИМУЛЯЦИИ ===")
    println("Количество участников: ${humans.size}")
    println("Время симуляции: $simulationTime секунд")
    println("=" * 50)

    // Основной цикл симуляции
    for (second in 1..simulationTime) {
        println("\n--- Секунда $second ---")

        // Каждый человек делает шаг
        humans.forEach { human ->
            human.move(timeStep)
        }

        // Небольшая пауза для наглядности
        Thread.sleep(500)
    }

    println("\n" + "=" * 50)
    println("=== КОНЕЦ СИМУЛЯЦИИ ===")
    println("\nФинальные позиции:")
    println("-".repeat(60))

    // Выводим финальные позиции всех участников
    humans.forEachIndexed { index, human ->
        val pos = human.getPosition()
        println("${index + 1}. ${human.getFullName()}")
        println("   Позиция: (${"%.2f".format(pos.x)}, ${"%.2f".format(pos.y)})")
        println("   Пройденное расстояние: ${"%.2f".format(sqrt(pos.x * pos.x + pos.y * pos.y))} м")
        println("-".repeat(40))
    }
}