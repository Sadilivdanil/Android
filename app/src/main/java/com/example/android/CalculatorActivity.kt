package com.example.android

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.widget.Button
import android.widget.TextView
import java.util.Locale

class CalculatorActivity : AppCompatActivity() {

    private lateinit var displayText: TextView
    private var currentInput = StringBuilder()
    private var hasOperator = false
    private var lastResult: String? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_calculator)

        displayText = findViewById(R.id.displayText)

        setupNumberButton(R.id.btn0, "0")
        setupNumberButton(R.id.btn1, "1")
        setupNumberButton(R.id.btn2, "2")
        setupNumberButton(R.id.btn3, "3")
        setupNumberButton(R.id.btn4, "4")
        setupNumberButton(R.id.btn5, "5")
        setupNumberButton(R.id.btn6, "6")
        setupNumberButton(R.id.btn7, "7")
        setupNumberButton(R.id.btn8, "8")
        setupNumberButton(R.id.btn9, "9")

        setupOperationButton(R.id.btnAdd, "+")
        setupOperationButton(R.id.btnSubtract, "-")
        setupOperationButton(R.id.btnMultiply, "*")
        setupOperationButton(R.id.btnDivide, "/")

        findViewById<Button>(R.id.btnEquals).setOnClickListener { calculateResult() }
        findViewById<Button>(R.id.btnClear).setOnClickListener { clearDisplay() }
        findViewById<Button>(R.id.btnDecimal).setOnClickListener { addDecimalPoint() }
        findViewById<Button>(R.id.btnBackspace).setOnClickListener { removeLastCharacter() }

        displayText.text = "0"
    }

    private fun setupNumberButton(buttonId: Int, number: String) {
        findViewById<Button>(buttonId).setOnClickListener {
            if (lastResult != null) {
                currentInput.clear()
                lastResult = null
            }
            currentInput.append(number)
            updateDisplay()
        }
    }

    private fun setupOperationButton(buttonId: Int, operator: String) {
        findViewById<Button>(buttonId).setOnClickListener {
            if (currentInput.isNotEmpty() && !hasOperator) {
                currentInput.append(operator)
                hasOperator = true
                updateDisplay()
            }
        }
    }

    private fun addDecimalPoint() {
        val lastNumber = getLastNumber()
        if (!lastNumber.contains(".")) {
            if (currentInput.isEmpty() || isOperator(currentInput.last())) {
                currentInput.append("0.")
            } else {
                currentInput.append(".")
            }
            updateDisplay()
        }
    }

    private fun removeLastCharacter() {
        if (currentInput.isNotEmpty()) {
            val lastChar = currentInput.last()
            if (isOperator(lastChar)) {
                hasOperator = false
            }
            currentInput.deleteCharAt(currentInput.length - 1)
            updateDisplay()
        }
    }

    private fun clearDisplay() {
        currentInput.clear()
        hasOperator = false
        lastResult = null
        displayText.text = "0"
    }

    private fun calculateResult() {
        if (currentInput.isNotEmpty() && hasOperator) {
            try {
                val expression = currentInput.toString()
                val result = evaluateExpression(expression)

                lastResult = if (result % 1 == 0.0) {
                    result.toInt().toString()
                } else {
                    String.format(Locale.getDefault(), "%.2f", result)
                }

                displayText.text = lastResult
                currentInput.clear()
                currentInput.append(lastResult)
                hasOperator = false

            } catch (e: Exception) {
                displayText.text = "Ошибка"
                currentInput.clear()
                hasOperator = false
            }
        } else {
            displayText.text = "Введите выражение"
        }
    }

    private fun evaluateExpression(expression: String): Double {
        val operators = listOf('+', '-', '*', '/')
        var operator = '+'
        var operatorFound = false

        for (op in operators) {
            if (expression.contains(op)) {
                operator = op
                operatorFound = true
                break
            }
        }

        if (!operatorFound) {
            return expression.toDouble()
        }

        val parts = expression.split(operator)
        if (parts.size != 2) {
            throw IllegalArgumentException("Неверное выражение")
        }

        val num1 = parts[0].toDouble()
        val num2 = parts[1].toDouble()

        return when (operator) {
            '+' -> num1 + num2
            '-' -> num1 - num2
            '*' -> num1 * num2
            '/' -> {
                if (num2 == 0.0) throw ArithmeticException("Деление на ноль")
                num1 / num2
            }
            else -> throw IllegalArgumentException("Неизвестная операция")
        }
    }

    private fun updateDisplay() {
        displayText.text = if (currentInput.isEmpty()) "0" else currentInput.toString()
    }

    private fun isOperator(char: Char): Boolean {
        return char == '+' || char == '-' || char == '*' || char == '/'
    }

    private fun getLastNumber(): String {
        var i = currentInput.length - 1
        while (i >= 0 && !isOperator(currentInput[i])) {
            i--
        }
        return currentInput.substring(i + 1)
    }
}