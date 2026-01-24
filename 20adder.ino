// 20adder.ino
// 2^23 + 2^23 -1 までの計算結果を4桁7セグ×2に表示させるためのプログラム

// ピン番号, 用途の定義
constexpr uint8_t PIN_A = 6;
constexpr uint8_t PIN_D = 7;
constexpr uint8_t PIN_C = 8;
constexpr uint8_t PIN_B = 9;
constexpr uint8_t PIN_DATA = 10;
constexpr uint8_t PIN_SL = 11;
constexpr uint8_t PIN_CLK = 12;
constexpr uint8_t PIN_8TH_DIGIT = 14;
constexpr uint8_t PIN_7TH_DIGIT = 15;
constexpr uint8_t PIN_6TH_DIGIT = 16;
constexpr uint8_t PIN_5TH_DIGIT = 17;
constexpr uint8_t PIN_4TH_DIGIT = 18;
constexpr uint8_t PIN_3RD_DIGIT = 19;
constexpr uint8_t PIN_2ND_DIGIT = 20;
constexpr uint8_t PIN_1ST_DIGIT = 21;
constexpr uint8_t digit_pins[8] = {
    PIN_1ST_DIGIT, PIN_2ND_DIGIT, PIN_3RD_DIGIT, PIN_4TH_DIGIT,
    PIN_5TH_DIGIT, PIN_6TH_DIGIT, PIN_7TH_DIGIT, PIN_8TH_DIGIT};

// 定数
constexpr uint8_t length = 24;         // 入力ビット数
constexpr uint8_t segments_length = 8; // 7セグの表示桁数

// 関数は配列を返せないので, 構造体に入れ込む
struct segments
{
  // std::vector<int> segment(segments_length);
  // にしたかったが, arduinoはC++標準ライブラリが使えなかった
  uint8_t segment[8]; // 配列の長さはsegments_length と同値
};

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    ; // Nano Every（USB CDC）では安定のため待つ
  }
  pinMode(PIN_A, OUTPUT);
  pinMode(PIN_D, OUTPUT);
  pinMode(PIN_C, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  pinMode(PIN_DATA, INPUT);
  pinMode(PIN_SL, OUTPUT);
  pinMode(PIN_CLK, OUTPUT);
  pinMode(PIN_8TH_DIGIT, OUTPUT);
  pinMode(PIN_7TH_DIGIT, OUTPUT);
  pinMode(PIN_6TH_DIGIT, OUTPUT);
  pinMode(PIN_5TH_DIGIT, OUTPUT);
  pinMode(PIN_4TH_DIGIT, OUTPUT);
  pinMode(PIN_3RD_DIGIT, OUTPUT);
  pinMode(PIN_2ND_DIGIT, OUTPUT);
  pinMode(PIN_1ST_DIGIT, OUTPUT);
  digitalWrite(PIN_SL, HIGH);
}

// クロック立ち上げ
void clk_rising()
{
  digitalWrite(PIN_CLK, HIGH);
  delayMicroseconds(1);
  digitalWrite(PIN_CLK, LOW);
}

uint32_t read_data()
{
  uint32_t val = 0;

  // 74HC165の内部に現在の値を保持させるためのクロック信号
  digitalWrite(PIN_SL, LOW);
  delayMicroseconds(1);
  digitalWrite(PIN_SL, HIGH);

  // 24bitの読みとり
  // 74HC165の出力はMLB→LSBで行われる
  for (uint8_t i = 0; i < length; i++)
  {
    val <<= 1;                         // 1bit左シフト
    val = val | digitalRead(PIN_DATA); // 現在の入力をLSBに代入
    clk_rising();
  }

  return val;
}

// 桁数を求める
uint8_t digits10(uint32_t val)
{
  if (val == 0)
  {
    return 1;
  }

  uint8_t cnt = 0;
  while (val > 0)
  {
    val /= 10;
    cnt++;
  }
  // 桁オーバーの対策
  if (cnt > segments_length)
  {
    cnt = segments_length;
  }
  return cnt;
}

// 10進数の入力データを1桁ずつ抜き出す
segments pullout_digit(uint32_t val, uint8_t digits_length)
{
  segments result = {};
  uint8_t least_digit;

  for (uint8_t i = 0; i < digits_length; i++)
  {
    // 1桁目を抜き出す
    least_digit = val % 10;
    result.segment[i] = least_digit;
    val = val / 10; // 位を下げる
  }

  return result;
}

// ダイナミック点灯で表示
// segments& は参照渡し(そのまま渡すとコピーになってメモリ消費が大きくなる)
void display(const segments &digits, uint8_t digits_length)
{
  uint8_t number;
  uint8_t illuminate_digits;
  for (uint8_t i = 0; i < digits_length; i++)
  {
    number = digits.segment[i];
    digitalWrite(PIN_A, number & 0b0001);
    digitalWrite(PIN_B, number & 0b0010);
    digitalWrite(PIN_C, number & 0b0100);
    digitalWrite(PIN_D, number & 0b1000);
    illuminate_digits = digit_pins[i];
    digitalWrite(illuminate_digits, HIGH);
    delayMicroseconds(100);
    // なぜか出力ピンをリセットしてあげないと次の出力に影響が出るパターン有り
    digitalWrite(PIN_A, LOW);
    digitalWrite(PIN_B, LOW);
    digitalWrite(PIN_C, LOW);
    digitalWrite(PIN_D, LOW);
    digitalWrite(illuminate_digits, LOW);
  }

  // 上位桁が0であった場合は表示しないが, 明るさは一定にするため, 周期を合わせる
  for (uint8_t i = digits_length; i < segments_length; i++)
  {
    delayMicroseconds(100);
  }
}

void loop()
{
  uint32_t input_bits = 0;
  uint32_t output_bits = 0;
  segments output_digits;
  uint8_t digits_length;
  input_bits = read_data();
  digits_length = digits10(input_bits);
  output_digits = pullout_digit(input_bits, digits_length);

  display(output_digits, digits_length);
}