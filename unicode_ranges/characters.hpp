#ifndef UTF8_RANGES_CHARACTERS_HPP
#define UTF8_RANGES_CHARACTERS_HPP

#include "utf8_char.hpp"
#include "utf16_char.hpp"
#include "utf32_char.hpp"

namespace unicode_ranges::characters
{
using namespace unicode_ranges::literals;

namespace utf8
{
namespace punctuation
{
inline constexpr utf8_char bullet = "•"_u8c;
inline constexpr utf8_char middle_dot = "·"_u8c;
inline constexpr utf8_char ellipsis = "…"_u8c;
inline constexpr utf8_char en_dash = "–"_u8c;
inline constexpr utf8_char em_dash = "—"_u8c;
inline constexpr utf8_char left_single_quotation_mark = "‘"_u8c;
inline constexpr utf8_char right_single_quotation_mark = "’"_u8c;
inline constexpr utf8_char left_double_quotation_mark = "“"_u8c;
inline constexpr utf8_char right_double_quotation_mark = "”"_u8c;
inline constexpr utf8_char inverted_exclamation_mark = "¡"_u8c;
inline constexpr utf8_char inverted_question_mark = "¿"_u8c;
}

namespace symbols
{
inline constexpr utf8_char copyright_sign = "©"_u8c;
inline constexpr utf8_char registered_sign = "®"_u8c;
inline constexpr utf8_char trade_mark_sign = "™"_u8c;
inline constexpr utf8_char section_sign = "§"_u8c;
inline constexpr utf8_char paragraph_sign = "¶"_u8c;
inline constexpr utf8_char numero_sign = "№"_u8c;
inline constexpr utf8_char check_mark = "✓"_u8c;
inline constexpr utf8_char cross_mark = "✗"_u8c;
inline constexpr utf8_char warning_sign = "⚠"_u8c;
}

namespace currency
{
inline constexpr utf8_char cent_sign = "¢"_u8c;
inline constexpr utf8_char pound_sign = "£"_u8c;
inline constexpr utf8_char euro_sign = "€"_u8c;
inline constexpr utf8_char yen_sign = "¥"_u8c;
inline constexpr utf8_char bitcoin_sign = "₿"_u8c;
}

namespace math
{
inline constexpr utf8_char degree_sign = "°"_u8c;
inline constexpr utf8_char plus_minus_sign = "±"_u8c;
inline constexpr utf8_char multiplication_sign = "×"_u8c;
inline constexpr utf8_char division_sign = "÷"_u8c;
inline constexpr utf8_char approximately_equal_to = "≈"_u8c;
inline constexpr utf8_char not_equal_to = "≠"_u8c;
inline constexpr utf8_char less_than_or_equal_to = "≤"_u8c;
inline constexpr utf8_char greater_than_or_equal_to = "≥"_u8c;
inline constexpr utf8_char infinity = "∞"_u8c;
}

namespace arrows
{
inline constexpr utf8_char left_arrow = "←"_u8c;
inline constexpr utf8_char up_arrow = "↑"_u8c;
inline constexpr utf8_char right_arrow = "→"_u8c;
inline constexpr utf8_char down_arrow = "↓"_u8c;
inline constexpr utf8_char left_right_arrow = "↔"_u8c;
inline constexpr utf8_char up_down_arrow = "↕"_u8c;
}

namespace emojis
{
inline constexpr utf8_char grinning_face = "😀"_u8c;
inline constexpr utf8_char beaming_face_with_smiling_eyes = "😁"_u8c;
inline constexpr utf8_char face_with_tears_of_joy = "😂"_u8c;
inline constexpr utf8_char rolling_on_the_floor_laughing = "🤣"_u8c;
inline constexpr utf8_char grinning_face_with_big_eyes = "😃"_u8c;
inline constexpr utf8_char grinning_face_with_smiling_eyes = "😄"_u8c;
inline constexpr utf8_char grinning_face_with_sweat = "😅"_u8c;
inline constexpr utf8_char smiling_face_with_open_hands = "🤗"_u8c;
inline constexpr utf8_char slightly_smiling_face = "🙂"_u8c;
inline constexpr utf8_char upside_down_face = "🙃"_u8c;
inline constexpr utf8_char winking_face = "😉"_u8c;
inline constexpr utf8_char smiling_face_with_smiling_eyes = "😊"_u8c;
inline constexpr utf8_char smiling_face_with_halo = "😇"_u8c;
inline constexpr utf8_char smiling_face_with_hearts = "🥰"_u8c;
inline constexpr utf8_char smiling_face_with_heart_eyes = "😍"_u8c;
inline constexpr utf8_char star_struck = "🤩"_u8c;
inline constexpr utf8_char face_blowing_a_kiss = "😘"_u8c;
inline constexpr utf8_char thinking_face = "🤔"_u8c;
inline constexpr utf8_char saluting_face = "🫡"_u8c;
inline constexpr utf8_char zipper_mouth_face = "🤐"_u8c;
inline constexpr utf8_char shushing_face = "🤫"_u8c;
inline constexpr utf8_char neutral_face = "😐"_u8c;
inline constexpr utf8_char expressionless_face = "😑"_u8c;
inline constexpr utf8_char smirking_face = "😏"_u8c;
inline constexpr utf8_char face_with_raised_eyebrow = "🤨"_u8c;
inline constexpr utf8_char pensive_face = "😔"_u8c;
inline constexpr utf8_char crying_face = "😢"_u8c;
inline constexpr utf8_char loudly_crying_face = "😭"_u8c;
inline constexpr utf8_char angry_face = "😠"_u8c;
inline constexpr utf8_char pouting_face = "😡"_u8c;
inline constexpr utf8_char face_with_symbols_on_mouth = "🤬"_u8c;
inline constexpr utf8_char anxious_face_with_sweat = "😰"_u8c;
inline constexpr utf8_char face_screaming_in_fear = "😱"_u8c;
inline constexpr utf8_char astonished_face = "😲"_u8c;
inline constexpr utf8_char sleeping_face = "😴"_u8c;
inline constexpr utf8_char yawning_face = "🥱"_u8c;
inline constexpr utf8_char nauseated_face = "🤢"_u8c;
inline constexpr utf8_char face_vomiting = "🤮"_u8c;
inline constexpr utf8_char face_with_medical_mask = "😷"_u8c;
inline constexpr utf8_char hot_face = "🥵"_u8c;
inline constexpr utf8_char cold_face = "🥶"_u8c;
inline constexpr utf8_char clown_face = "🤡"_u8c;
inline constexpr utf8_char cowboy_hat_face = "🤠"_u8c;
inline constexpr utf8_char partying_face = "🥳"_u8c;
inline constexpr utf8_char nerd_face = "🤓"_u8c;
inline constexpr utf8_char skull = "💀"_u8c;
inline constexpr utf8_char ghost = "👻"_u8c;
inline constexpr utf8_char alien = "👽"_u8c;
inline constexpr utf8_char robot = "🤖"_u8c;
inline constexpr utf8_char pile_of_poo = "💩"_u8c;
inline constexpr utf8_char waving_hand = "👋"_u8c;
inline constexpr utf8_char raised_back_of_hand = "🤚"_u8c;
inline constexpr utf8_char raised_hand = "✋"_u8c;
inline constexpr utf8_char vulcan_salute = "🖖"_u8c;
inline constexpr utf8_char ok_hand = "👌"_u8c;
inline constexpr utf8_char pinched_fingers = "🤌"_u8c;
inline constexpr utf8_char pinching_hand = "🤏"_u8c;
inline constexpr utf8_char victory_hand = "✌"_u8c;
inline constexpr utf8_char crossed_fingers = "🤞"_u8c;
inline constexpr utf8_char love_you_gesture = "🤟"_u8c;
inline constexpr utf8_char call_me_hand = "🤙"_u8c;
inline constexpr utf8_char backhand_index_pointing_up = "👆"_u8c;
inline constexpr utf8_char thumbs_up = "👍"_u8c;
inline constexpr utf8_char thumbs_down = "👎"_u8c;
inline constexpr utf8_char clapping_hands = "👏"_u8c;
inline constexpr utf8_char raising_hands = "🙌"_u8c;
inline constexpr utf8_char folded_hands = "🙏"_u8c;
inline constexpr utf8_char flexed_biceps = "💪"_u8c;
inline constexpr utf8_char mechanical_arm = "🦾"_u8c;
inline constexpr utf8_char handshake = "🤝"_u8c;
inline constexpr utf8_char red_heart = "❤"_u8c;
inline constexpr utf8_char orange_heart = "🧡"_u8c;
inline constexpr utf8_char yellow_heart = "💛"_u8c;
inline constexpr utf8_char green_heart = "💚"_u8c;
inline constexpr utf8_char blue_heart = "💙"_u8c;
inline constexpr utf8_char purple_heart = "💜"_u8c;
inline constexpr utf8_char black_heart = "🖤"_u8c;
inline constexpr utf8_char white_heart = "🤍"_u8c;
inline constexpr utf8_char brown_heart = "🤎"_u8c;
inline constexpr utf8_char broken_heart = "💔"_u8c;
inline constexpr utf8_char sparkles = "✨"_u8c;
inline constexpr utf8_char fire = "🔥"_u8c;
inline constexpr utf8_char collision = "💥"_u8c;
inline constexpr utf8_char hundred_points = "💯"_u8c;
inline constexpr utf8_char party_popper = "🎉"_u8c;
inline constexpr utf8_char wrapped_gift = "🎁"_u8c;
inline constexpr utf8_char balloon = "🎈"_u8c;
inline constexpr utf8_char dog_face = "🐶"_u8c;
inline constexpr utf8_char cat_face = "🐱"_u8c;
inline constexpr utf8_char mouse_face = "🐭"_u8c;
inline constexpr utf8_char rabbit_face = "🐰"_u8c;
inline constexpr utf8_char fox = "🦊"_u8c;
inline constexpr utf8_char bear = "🐻"_u8c;
inline constexpr utf8_char panda = "🐼"_u8c;
inline constexpr utf8_char koala = "🐨"_u8c;
inline constexpr utf8_char tiger_face = "🐯"_u8c;
inline constexpr utf8_char lion = "🦁"_u8c;
inline constexpr utf8_char cow_face = "🐮"_u8c;
inline constexpr utf8_char pig_face = "🐷"_u8c;
inline constexpr utf8_char frog = "🐸"_u8c;
inline constexpr utf8_char monkey_face = "🐵"_u8c;
inline constexpr utf8_char chicken = "🐔"_u8c;
inline constexpr utf8_char penguin = "🐧"_u8c;
inline constexpr utf8_char bird = "🐦"_u8c;
inline constexpr utf8_char owl = "🦉"_u8c;
inline constexpr utf8_char unicorn = "🦄"_u8c;
inline constexpr utf8_char honeybee = "🐝"_u8c;
inline constexpr utf8_char butterfly = "🦋"_u8c;
inline constexpr utf8_char lady_beetle = "🐞"_u8c;
inline constexpr utf8_char turtle = "🐢"_u8c;
inline constexpr utf8_char snake = "🐍"_u8c;
inline constexpr utf8_char dragon_face = "🐲"_u8c;
inline constexpr utf8_char cactus = "🌵"_u8c;
inline constexpr utf8_char four_leaf_clover = "🍀"_u8c;
inline constexpr utf8_char maple_leaf = "🍁"_u8c;
inline constexpr utf8_char leaf_fluttering_in_wind = "🍃"_u8c;
inline constexpr utf8_char rose = "🌹"_u8c;
inline constexpr utf8_char sunflower = "🌻"_u8c;
inline constexpr utf8_char mushroom = "🍄"_u8c;
inline constexpr utf8_char globe_showing_europe_africa = "🌍"_u8c;
inline constexpr utf8_char sun = "☀"_u8c;
inline constexpr utf8_char cloud = "☁"_u8c;
inline constexpr utf8_char rainbow = "🌈"_u8c;
inline constexpr utf8_char snowflake = "❄"_u8c;
inline constexpr utf8_char red_apple = "🍎"_u8c;
inline constexpr utf8_char green_apple = "🍏"_u8c;
inline constexpr utf8_char pear = "🍐"_u8c;
inline constexpr utf8_char peach = "🍑"_u8c;
inline constexpr utf8_char cherries = "🍒"_u8c;
inline constexpr utf8_char strawberry = "🍓"_u8c;
inline constexpr utf8_char blueberries = "🫐"_u8c;
inline constexpr utf8_char kiwi_fruit = "🥝"_u8c;
inline constexpr utf8_char watermelon = "🍉"_u8c;
inline constexpr utf8_char lemon = "🍋"_u8c;
inline constexpr utf8_char banana = "🍌"_u8c;
inline constexpr utf8_char pineapple = "🍍"_u8c;
inline constexpr utf8_char mango = "🥭"_u8c;
inline constexpr utf8_char grapes = "🍇"_u8c;
inline constexpr utf8_char avocado = "🥑"_u8c;
inline constexpr utf8_char carrot = "🥕"_u8c;
inline constexpr utf8_char hot_pepper = "🌶"_u8c;
inline constexpr utf8_char ear_of_corn = "🌽"_u8c;
inline constexpr utf8_char croissant = "🥐"_u8c;
inline constexpr utf8_char bread = "🍞"_u8c;
inline constexpr utf8_char pretzel = "🥨"_u8c;
inline constexpr utf8_char cheese_wedge = "🧀"_u8c;
inline constexpr utf8_char hamburger = "🍔"_u8c;
inline constexpr utf8_char pizza = "🍕"_u8c;
inline constexpr utf8_char french_fries = "🍟"_u8c;
inline constexpr utf8_char taco = "🌮"_u8c;
inline constexpr utf8_char burrito = "🌯"_u8c;
inline constexpr utf8_char sushi = "🍣"_u8c;
inline constexpr utf8_char steaming_bowl = "🍜"_u8c;
inline constexpr utf8_char birthday_cake = "🎂"_u8c;
inline constexpr utf8_char doughnut = "🍩"_u8c;
inline constexpr utf8_char cookie = "🍪"_u8c;
inline constexpr utf8_char soft_ice_cream = "🍦"_u8c;
inline constexpr utf8_char hot_beverage = "☕"_u8c;
inline constexpr utf8_char beer_mug = "🍺"_u8c;
inline constexpr utf8_char clinking_beer_mugs = "🍻"_u8c;
inline constexpr utf8_char wine_glass = "🍷"_u8c;
inline constexpr utf8_char cocktail_glass = "🍸"_u8c;
inline constexpr utf8_char tropical_drink = "🍹"_u8c;
inline constexpr utf8_char soccer_ball = "⚽"_u8c;
inline constexpr utf8_char basketball = "🏀"_u8c;
inline constexpr utf8_char volleyball = "🏐"_u8c;
inline constexpr utf8_char baseball = "⚾"_u8c;
inline constexpr utf8_char tennis = "🎾"_u8c;
inline constexpr utf8_char trophy = "🏆"_u8c;
inline constexpr utf8_char first_place_medal = "🥇"_u8c;
inline constexpr utf8_char artist_palette = "🎨"_u8c;
inline constexpr utf8_char musical_note = "🎵"_u8c;
inline constexpr utf8_char headphones = "🎧"_u8c;
inline constexpr utf8_char studio_microphone = "🎙"_u8c;
inline constexpr utf8_char movie_camera = "🎥"_u8c;
inline constexpr utf8_char camera = "📷"_u8c;
inline constexpr utf8_char video_camera = "📹"_u8c;
inline constexpr utf8_char clapper_board = "🎬"_u8c;
inline constexpr utf8_char light_bulb = "💡"_u8c;
inline constexpr utf8_char flashlight = "🔦"_u8c;
inline constexpr utf8_char magnifying_glass_tilted_left = "🔍"_u8c;
inline constexpr utf8_char link = "🔗"_u8c;
inline constexpr utf8_char paperclip = "📎"_u8c;
inline constexpr utf8_char pushpin = "📌"_u8c;
inline constexpr utf8_char lock = "🔒"_u8c;
inline constexpr utf8_char key = "🔑"_u8c;
inline constexpr utf8_char hammer = "🔨"_u8c;
inline constexpr utf8_char wrench = "🔧"_u8c;
inline constexpr utf8_char gear = "⚙"_u8c;
inline constexpr utf8_char laptop = "💻"_u8c;
inline constexpr utf8_char mobile_phone = "📱"_u8c;
inline constexpr utf8_char package = "📦"_u8c;
inline constexpr utf8_char shopping_cart = "🛒"_u8c;
inline constexpr utf8_char money_bag = "💰"_u8c;
inline constexpr utf8_char credit_card = "💳"_u8c;
inline constexpr utf8_char gem_stone = "💎"_u8c;
inline constexpr utf8_char automobile = "🚗"_u8c;
inline constexpr utf8_char taxi = "🚕"_u8c;
inline constexpr utf8_char bus = "🚌"_u8c;
inline constexpr utf8_char police_car = "🚓"_u8c;
inline constexpr utf8_char racing_car = "🏎"_u8c;
inline constexpr utf8_char motorcycle = "🏍"_u8c;
inline constexpr utf8_char bicycle = "🚲"_u8c;
inline constexpr utf8_char kick_scooter = "🛴"_u8c;
inline constexpr utf8_char airplane = "✈"_u8c;
inline constexpr utf8_char rocket = "🚀"_u8c;
inline constexpr utf8_char helicopter = "🚁"_u8c;
inline constexpr utf8_char sailboat = "⛵"_u8c;
inline constexpr utf8_char anchor = "⚓"_u8c;
inline constexpr utf8_char fuel_pump = "⛽"_u8c;
inline constexpr utf8_char house = "🏠"_u8c;
inline constexpr utf8_char office_building = "🏢"_u8c;
inline constexpr utf8_char hospital = "🏥"_u8c;
inline constexpr utf8_char school = "🏫"_u8c;
inline constexpr utf8_char bank = "🏦"_u8c;
inline constexpr utf8_char hotel = "🏨"_u8c;
inline constexpr utf8_char stadium = "🏟"_u8c;
inline constexpr utf8_char mountain = "⛰"_u8c;
inline constexpr utf8_char volcano = "🌋"_u8c;
inline constexpr utf8_char beach_with_umbrella = "🏖"_u8c;
inline constexpr utf8_char tent = "⛺"_u8c;
}
}

namespace utf16
{
namespace punctuation
{
inline constexpr utf16_char bullet = u"•"_u16c;
inline constexpr utf16_char middle_dot = u"·"_u16c;
inline constexpr utf16_char ellipsis = u"…"_u16c;
inline constexpr utf16_char en_dash = u"–"_u16c;
inline constexpr utf16_char em_dash = u"—"_u16c;
inline constexpr utf16_char left_single_quotation_mark = u"‘"_u16c;
inline constexpr utf16_char right_single_quotation_mark = u"’"_u16c;
inline constexpr utf16_char left_double_quotation_mark = u"“"_u16c;
inline constexpr utf16_char right_double_quotation_mark = u"”"_u16c;
inline constexpr utf16_char inverted_exclamation_mark = u"¡"_u16c;
inline constexpr utf16_char inverted_question_mark = u"¿"_u16c;
}

namespace symbols
{
inline constexpr utf16_char copyright_sign = u"©"_u16c;
inline constexpr utf16_char registered_sign = u"®"_u16c;
inline constexpr utf16_char trade_mark_sign = u"™"_u16c;
inline constexpr utf16_char section_sign = u"§"_u16c;
inline constexpr utf16_char paragraph_sign = u"¶"_u16c;
inline constexpr utf16_char numero_sign = u"№"_u16c;
inline constexpr utf16_char check_mark = u"✓"_u16c;
inline constexpr utf16_char cross_mark = u"✗"_u16c;
inline constexpr utf16_char warning_sign = u"⚠"_u16c;
}

namespace currency
{
inline constexpr utf16_char cent_sign = u"¢"_u16c;
inline constexpr utf16_char pound_sign = u"£"_u16c;
inline constexpr utf16_char euro_sign = u"€"_u16c;
inline constexpr utf16_char yen_sign = u"¥"_u16c;
inline constexpr utf16_char bitcoin_sign = u"₿"_u16c;
}

namespace math
{
inline constexpr utf16_char degree_sign = u"°"_u16c;
inline constexpr utf16_char plus_minus_sign = u"±"_u16c;
inline constexpr utf16_char multiplication_sign = u"×"_u16c;
inline constexpr utf16_char division_sign = u"÷"_u16c;
inline constexpr utf16_char approximately_equal_to = u"≈"_u16c;
inline constexpr utf16_char not_equal_to = u"≠"_u16c;
inline constexpr utf16_char less_than_or_equal_to = u"≤"_u16c;
inline constexpr utf16_char greater_than_or_equal_to = u"≥"_u16c;
inline constexpr utf16_char infinity = u"∞"_u16c;
}

namespace arrows
{
inline constexpr utf16_char left_arrow = u"←"_u16c;
inline constexpr utf16_char up_arrow = u"↑"_u16c;
inline constexpr utf16_char right_arrow = u"→"_u16c;
inline constexpr utf16_char down_arrow = u"↓"_u16c;
inline constexpr utf16_char left_right_arrow = u"↔"_u16c;
inline constexpr utf16_char up_down_arrow = u"↕"_u16c;
}

namespace emojis
{
inline constexpr utf16_char grinning_face = u"😀"_u16c;
inline constexpr utf16_char beaming_face_with_smiling_eyes = u"😁"_u16c;
inline constexpr utf16_char face_with_tears_of_joy = u"😂"_u16c;
inline constexpr utf16_char rolling_on_the_floor_laughing = u"🤣"_u16c;
inline constexpr utf16_char grinning_face_with_big_eyes = u"😃"_u16c;
inline constexpr utf16_char grinning_face_with_smiling_eyes = u"😄"_u16c;
inline constexpr utf16_char grinning_face_with_sweat = u"😅"_u16c;
inline constexpr utf16_char smiling_face_with_open_hands = u"🤗"_u16c;
inline constexpr utf16_char slightly_smiling_face = u"🙂"_u16c;
inline constexpr utf16_char upside_down_face = u"🙃"_u16c;
inline constexpr utf16_char winking_face = u"😉"_u16c;
inline constexpr utf16_char smiling_face_with_smiling_eyes = u"😊"_u16c;
inline constexpr utf16_char smiling_face_with_halo = u"😇"_u16c;
inline constexpr utf16_char smiling_face_with_hearts = u"🥰"_u16c;
inline constexpr utf16_char smiling_face_with_heart_eyes = u"😍"_u16c;
inline constexpr utf16_char star_struck = u"🤩"_u16c;
inline constexpr utf16_char face_blowing_a_kiss = u"😘"_u16c;
inline constexpr utf16_char thinking_face = u"🤔"_u16c;
inline constexpr utf16_char saluting_face = u"🫡"_u16c;
inline constexpr utf16_char zipper_mouth_face = u"🤐"_u16c;
inline constexpr utf16_char shushing_face = u"🤫"_u16c;
inline constexpr utf16_char neutral_face = u"😐"_u16c;
inline constexpr utf16_char expressionless_face = u"😑"_u16c;
inline constexpr utf16_char smirking_face = u"😏"_u16c;
inline constexpr utf16_char face_with_raised_eyebrow = u"🤨"_u16c;
inline constexpr utf16_char pensive_face = u"😔"_u16c;
inline constexpr utf16_char crying_face = u"😢"_u16c;
inline constexpr utf16_char loudly_crying_face = u"😭"_u16c;
inline constexpr utf16_char angry_face = u"😠"_u16c;
inline constexpr utf16_char pouting_face = u"😡"_u16c;
inline constexpr utf16_char face_with_symbols_on_mouth = u"🤬"_u16c;
inline constexpr utf16_char anxious_face_with_sweat = u"😰"_u16c;
inline constexpr utf16_char face_screaming_in_fear = u"😱"_u16c;
inline constexpr utf16_char astonished_face = u"😲"_u16c;
inline constexpr utf16_char sleeping_face = u"😴"_u16c;
inline constexpr utf16_char yawning_face = u"🥱"_u16c;
inline constexpr utf16_char nauseated_face = u"🤢"_u16c;
inline constexpr utf16_char face_vomiting = u"🤮"_u16c;
inline constexpr utf16_char face_with_medical_mask = u"😷"_u16c;
inline constexpr utf16_char hot_face = u"🥵"_u16c;
inline constexpr utf16_char cold_face = u"🥶"_u16c;
inline constexpr utf16_char clown_face = u"🤡"_u16c;
inline constexpr utf16_char cowboy_hat_face = u"🤠"_u16c;
inline constexpr utf16_char partying_face = u"🥳"_u16c;
inline constexpr utf16_char nerd_face = u"🤓"_u16c;
inline constexpr utf16_char skull = u"💀"_u16c;
inline constexpr utf16_char ghost = u"👻"_u16c;
inline constexpr utf16_char alien = u"👽"_u16c;
inline constexpr utf16_char robot = u"🤖"_u16c;
inline constexpr utf16_char pile_of_poo = u"💩"_u16c;
inline constexpr utf16_char waving_hand = u"👋"_u16c;
inline constexpr utf16_char raised_back_of_hand = u"🤚"_u16c;
inline constexpr utf16_char raised_hand = u"✋"_u16c;
inline constexpr utf16_char vulcan_salute = u"🖖"_u16c;
inline constexpr utf16_char ok_hand = u"👌"_u16c;
inline constexpr utf16_char pinched_fingers = u"🤌"_u16c;
inline constexpr utf16_char pinching_hand = u"🤏"_u16c;
inline constexpr utf16_char victory_hand = u"✌"_u16c;
inline constexpr utf16_char crossed_fingers = u"🤞"_u16c;
inline constexpr utf16_char love_you_gesture = u"🤟"_u16c;
inline constexpr utf16_char call_me_hand = u"🤙"_u16c;
inline constexpr utf16_char backhand_index_pointing_up = u"👆"_u16c;
inline constexpr utf16_char thumbs_up = u"👍"_u16c;
inline constexpr utf16_char thumbs_down = u"👎"_u16c;
inline constexpr utf16_char clapping_hands = u"👏"_u16c;
inline constexpr utf16_char raising_hands = u"🙌"_u16c;
inline constexpr utf16_char folded_hands = u"🙏"_u16c;
inline constexpr utf16_char flexed_biceps = u"💪"_u16c;
inline constexpr utf16_char mechanical_arm = u"🦾"_u16c;
inline constexpr utf16_char handshake = u"🤝"_u16c;
inline constexpr utf16_char red_heart = u"❤"_u16c;
inline constexpr utf16_char orange_heart = u"🧡"_u16c;
inline constexpr utf16_char yellow_heart = u"💛"_u16c;
inline constexpr utf16_char green_heart = u"💚"_u16c;
inline constexpr utf16_char blue_heart = u"💙"_u16c;
inline constexpr utf16_char purple_heart = u"💜"_u16c;
inline constexpr utf16_char black_heart = u"🖤"_u16c;
inline constexpr utf16_char white_heart = u"🤍"_u16c;
inline constexpr utf16_char brown_heart = u"🤎"_u16c;
inline constexpr utf16_char broken_heart = u"💔"_u16c;
inline constexpr utf16_char sparkles = u"✨"_u16c;
inline constexpr utf16_char fire = u"🔥"_u16c;
inline constexpr utf16_char collision = u"💥"_u16c;
inline constexpr utf16_char hundred_points = u"💯"_u16c;
inline constexpr utf16_char party_popper = u"🎉"_u16c;
inline constexpr utf16_char wrapped_gift = u"🎁"_u16c;
inline constexpr utf16_char balloon = u"🎈"_u16c;
inline constexpr utf16_char dog_face = u"🐶"_u16c;
inline constexpr utf16_char cat_face = u"🐱"_u16c;
inline constexpr utf16_char mouse_face = u"🐭"_u16c;
inline constexpr utf16_char rabbit_face = u"🐰"_u16c;
inline constexpr utf16_char fox = u"🦊"_u16c;
inline constexpr utf16_char bear = u"🐻"_u16c;
inline constexpr utf16_char panda = u"🐼"_u16c;
inline constexpr utf16_char koala = u"🐨"_u16c;
inline constexpr utf16_char tiger_face = u"🐯"_u16c;
inline constexpr utf16_char lion = u"🦁"_u16c;
inline constexpr utf16_char cow_face = u"🐮"_u16c;
inline constexpr utf16_char pig_face = u"🐷"_u16c;
inline constexpr utf16_char frog = u"🐸"_u16c;
inline constexpr utf16_char monkey_face = u"🐵"_u16c;
inline constexpr utf16_char chicken = u"🐔"_u16c;
inline constexpr utf16_char penguin = u"🐧"_u16c;
inline constexpr utf16_char bird = u"🐦"_u16c;
inline constexpr utf16_char owl = u"🦉"_u16c;
inline constexpr utf16_char unicorn = u"🦄"_u16c;
inline constexpr utf16_char honeybee = u"🐝"_u16c;
inline constexpr utf16_char butterfly = u"🦋"_u16c;
inline constexpr utf16_char lady_beetle = u"🐞"_u16c;
inline constexpr utf16_char turtle = u"🐢"_u16c;
inline constexpr utf16_char snake = u"🐍"_u16c;
inline constexpr utf16_char dragon_face = u"🐲"_u16c;
inline constexpr utf16_char cactus = u"🌵"_u16c;
inline constexpr utf16_char four_leaf_clover = u"🍀"_u16c;
inline constexpr utf16_char maple_leaf = u"🍁"_u16c;
inline constexpr utf16_char leaf_fluttering_in_wind = u"🍃"_u16c;
inline constexpr utf16_char rose = u"🌹"_u16c;
inline constexpr utf16_char sunflower = u"🌻"_u16c;
inline constexpr utf16_char mushroom = u"🍄"_u16c;
inline constexpr utf16_char globe_showing_europe_africa = u"🌍"_u16c;
inline constexpr utf16_char sun = u"☀"_u16c;
inline constexpr utf16_char cloud = u"☁"_u16c;
inline constexpr utf16_char rainbow = u"🌈"_u16c;
inline constexpr utf16_char snowflake = u"❄"_u16c;
inline constexpr utf16_char red_apple = u"🍎"_u16c;
inline constexpr utf16_char green_apple = u"🍏"_u16c;
inline constexpr utf16_char pear = u"🍐"_u16c;
inline constexpr utf16_char peach = u"🍑"_u16c;
inline constexpr utf16_char cherries = u"🍒"_u16c;
inline constexpr utf16_char strawberry = u"🍓"_u16c;
inline constexpr utf16_char blueberries = u"🫐"_u16c;
inline constexpr utf16_char kiwi_fruit = u"🥝"_u16c;
inline constexpr utf16_char watermelon = u"🍉"_u16c;
inline constexpr utf16_char lemon = u"🍋"_u16c;
inline constexpr utf16_char banana = u"🍌"_u16c;
inline constexpr utf16_char pineapple = u"🍍"_u16c;
inline constexpr utf16_char mango = u"🥭"_u16c;
inline constexpr utf16_char grapes = u"🍇"_u16c;
inline constexpr utf16_char avocado = u"🥑"_u16c;
inline constexpr utf16_char carrot = u"🥕"_u16c;
inline constexpr utf16_char hot_pepper = u"🌶"_u16c;
inline constexpr utf16_char ear_of_corn = u"🌽"_u16c;
inline constexpr utf16_char croissant = u"🥐"_u16c;
inline constexpr utf16_char bread = u"🍞"_u16c;
inline constexpr utf16_char pretzel = u"🥨"_u16c;
inline constexpr utf16_char cheese_wedge = u"🧀"_u16c;
inline constexpr utf16_char hamburger = u"🍔"_u16c;
inline constexpr utf16_char pizza = u"🍕"_u16c;
inline constexpr utf16_char french_fries = u"🍟"_u16c;
inline constexpr utf16_char taco = u"🌮"_u16c;
inline constexpr utf16_char burrito = u"🌯"_u16c;
inline constexpr utf16_char sushi = u"🍣"_u16c;
inline constexpr utf16_char steaming_bowl = u"🍜"_u16c;
inline constexpr utf16_char birthday_cake = u"🎂"_u16c;
inline constexpr utf16_char doughnut = u"🍩"_u16c;
inline constexpr utf16_char cookie = u"🍪"_u16c;
inline constexpr utf16_char soft_ice_cream = u"🍦"_u16c;
inline constexpr utf16_char hot_beverage = u"☕"_u16c;
inline constexpr utf16_char beer_mug = u"🍺"_u16c;
inline constexpr utf16_char clinking_beer_mugs = u"🍻"_u16c;
inline constexpr utf16_char wine_glass = u"🍷"_u16c;
inline constexpr utf16_char cocktail_glass = u"🍸"_u16c;
inline constexpr utf16_char tropical_drink = u"🍹"_u16c;
inline constexpr utf16_char soccer_ball = u"⚽"_u16c;
inline constexpr utf16_char basketball = u"🏀"_u16c;
inline constexpr utf16_char volleyball = u"🏐"_u16c;
inline constexpr utf16_char baseball = u"⚾"_u16c;
inline constexpr utf16_char tennis = u"🎾"_u16c;
inline constexpr utf16_char trophy = u"🏆"_u16c;
inline constexpr utf16_char first_place_medal = u"🥇"_u16c;
inline constexpr utf16_char artist_palette = u"🎨"_u16c;
inline constexpr utf16_char musical_note = u"🎵"_u16c;
inline constexpr utf16_char headphones = u"🎧"_u16c;
inline constexpr utf16_char studio_microphone = u"🎙"_u16c;
inline constexpr utf16_char movie_camera = u"🎥"_u16c;
inline constexpr utf16_char camera = u"📷"_u16c;
inline constexpr utf16_char video_camera = u"📹"_u16c;
inline constexpr utf16_char clapper_board = u"🎬"_u16c;
inline constexpr utf16_char light_bulb = u"💡"_u16c;
inline constexpr utf16_char flashlight = u"🔦"_u16c;
inline constexpr utf16_char magnifying_glass_tilted_left = u"🔍"_u16c;
inline constexpr utf16_char link = u"🔗"_u16c;
inline constexpr utf16_char paperclip = u"📎"_u16c;
inline constexpr utf16_char pushpin = u"📌"_u16c;
inline constexpr utf16_char lock = u"🔒"_u16c;
inline constexpr utf16_char key = u"🔑"_u16c;
inline constexpr utf16_char hammer = u"🔨"_u16c;
inline constexpr utf16_char wrench = u"🔧"_u16c;
inline constexpr utf16_char gear = u"⚙"_u16c;
inline constexpr utf16_char laptop = u"💻"_u16c;
inline constexpr utf16_char mobile_phone = u"📱"_u16c;
inline constexpr utf16_char package = u"📦"_u16c;
inline constexpr utf16_char shopping_cart = u"🛒"_u16c;
inline constexpr utf16_char money_bag = u"💰"_u16c;
inline constexpr utf16_char credit_card = u"💳"_u16c;
inline constexpr utf16_char gem_stone = u"💎"_u16c;
inline constexpr utf16_char automobile = u"🚗"_u16c;
inline constexpr utf16_char taxi = u"🚕"_u16c;
inline constexpr utf16_char bus = u"🚌"_u16c;
inline constexpr utf16_char police_car = u"🚓"_u16c;
inline constexpr utf16_char racing_car = u"🏎"_u16c;
inline constexpr utf16_char motorcycle = u"🏍"_u16c;
inline constexpr utf16_char bicycle = u"🚲"_u16c;
inline constexpr utf16_char kick_scooter = u"🛴"_u16c;
inline constexpr utf16_char airplane = u"✈"_u16c;
inline constexpr utf16_char rocket = u"🚀"_u16c;
inline constexpr utf16_char helicopter = u"🚁"_u16c;
inline constexpr utf16_char sailboat = u"⛵"_u16c;
inline constexpr utf16_char anchor = u"⚓"_u16c;
inline constexpr utf16_char fuel_pump = u"⛽"_u16c;
inline constexpr utf16_char house = u"🏠"_u16c;
inline constexpr utf16_char office_building = u"🏢"_u16c;
inline constexpr utf16_char hospital = u"🏥"_u16c;
inline constexpr utf16_char school = u"🏫"_u16c;
inline constexpr utf16_char bank = u"🏦"_u16c;
inline constexpr utf16_char hotel = u"🏨"_u16c;
inline constexpr utf16_char stadium = u"🏟"_u16c;
inline constexpr utf16_char mountain = u"⛰"_u16c;
inline constexpr utf16_char volcano = u"🌋"_u16c;
inline constexpr utf16_char beach_with_umbrella = u"🏖"_u16c;
inline constexpr utf16_char tent = u"⛺"_u16c;
}
}

namespace utf32
{
namespace punctuation
{
inline constexpr utf32_char bullet = U"•"_u32c;
inline constexpr utf32_char middle_dot = U"·"_u32c;
inline constexpr utf32_char ellipsis = U"…"_u32c;
inline constexpr utf32_char en_dash = U"–"_u32c;
inline constexpr utf32_char em_dash = U"—"_u32c;
inline constexpr utf32_char left_single_quotation_mark = U"‘"_u32c;
inline constexpr utf32_char right_single_quotation_mark = U"’"_u32c;
inline constexpr utf32_char left_double_quotation_mark = U"“"_u32c;
inline constexpr utf32_char right_double_quotation_mark = U"”"_u32c;
inline constexpr utf32_char inverted_exclamation_mark = U"¡"_u32c;
inline constexpr utf32_char inverted_question_mark = U"¿"_u32c;
}

namespace symbols
{
inline constexpr utf32_char copyright_sign = U"©"_u32c;
inline constexpr utf32_char registered_sign = U"®"_u32c;
inline constexpr utf32_char trade_mark_sign = U"™"_u32c;
inline constexpr utf32_char section_sign = U"§"_u32c;
inline constexpr utf32_char paragraph_sign = U"¶"_u32c;
inline constexpr utf32_char numero_sign = U"№"_u32c;
inline constexpr utf32_char check_mark = U"✓"_u32c;
inline constexpr utf32_char cross_mark = U"✗"_u32c;
inline constexpr utf32_char warning_sign = U"⚠"_u32c;
}

namespace currency
{
inline constexpr utf32_char cent_sign = U"¢"_u32c;
inline constexpr utf32_char pound_sign = U"£"_u32c;
inline constexpr utf32_char euro_sign = U"€"_u32c;
inline constexpr utf32_char yen_sign = U"¥"_u32c;
inline constexpr utf32_char bitcoin_sign = U"₿"_u32c;
}

namespace math
{
inline constexpr utf32_char degree_sign = U"°"_u32c;
inline constexpr utf32_char plus_minus_sign = U"±"_u32c;
inline constexpr utf32_char multiplication_sign = U"×"_u32c;
inline constexpr utf32_char division_sign = U"÷"_u32c;
inline constexpr utf32_char approximately_equal_to = U"≈"_u32c;
inline constexpr utf32_char not_equal_to = U"≠"_u32c;
inline constexpr utf32_char less_than_or_equal_to = U"≤"_u32c;
inline constexpr utf32_char greater_than_or_equal_to = U"≥"_u32c;
inline constexpr utf32_char infinity = U"∞"_u32c;
}

namespace arrows
{
inline constexpr utf32_char left_arrow = U"←"_u32c;
inline constexpr utf32_char up_arrow = U"↑"_u32c;
inline constexpr utf32_char right_arrow = U"→"_u32c;
inline constexpr utf32_char down_arrow = U"↓"_u32c;
inline constexpr utf32_char left_right_arrow = U"↔"_u32c;
inline constexpr utf32_char up_down_arrow = U"↕"_u32c;
}

namespace emojis
{
inline constexpr utf32_char grinning_face = U"😀"_u32c;
inline constexpr utf32_char beaming_face_with_smiling_eyes = U"😁"_u32c;
inline constexpr utf32_char face_with_tears_of_joy = U"😂"_u32c;
inline constexpr utf32_char rolling_on_the_floor_laughing = U"🤣"_u32c;
inline constexpr utf32_char grinning_face_with_big_eyes = U"😃"_u32c;
inline constexpr utf32_char grinning_face_with_smiling_eyes = U"😄"_u32c;
inline constexpr utf32_char grinning_face_with_sweat = U"😅"_u32c;
inline constexpr utf32_char smiling_face_with_open_hands = U"🤗"_u32c;
inline constexpr utf32_char slightly_smiling_face = U"🙂"_u32c;
inline constexpr utf32_char upside_down_face = U"🙃"_u32c;
inline constexpr utf32_char winking_face = U"😉"_u32c;
inline constexpr utf32_char smiling_face_with_smiling_eyes = U"😊"_u32c;
inline constexpr utf32_char smiling_face_with_halo = U"😇"_u32c;
inline constexpr utf32_char smiling_face_with_hearts = U"🥰"_u32c;
inline constexpr utf32_char smiling_face_with_heart_eyes = U"😍"_u32c;
inline constexpr utf32_char star_struck = U"🤩"_u32c;
inline constexpr utf32_char face_blowing_a_kiss = U"😘"_u32c;
inline constexpr utf32_char thinking_face = U"🤔"_u32c;
inline constexpr utf32_char saluting_face = U"🫡"_u32c;
inline constexpr utf32_char zipper_mouth_face = U"🤐"_u32c;
inline constexpr utf32_char shushing_face = U"🤫"_u32c;
inline constexpr utf32_char neutral_face = U"😐"_u32c;
inline constexpr utf32_char expressionless_face = U"😑"_u32c;
inline constexpr utf32_char smirking_face = U"😏"_u32c;
inline constexpr utf32_char face_with_raised_eyebrow = U"🤨"_u32c;
inline constexpr utf32_char pensive_face = U"😔"_u32c;
inline constexpr utf32_char crying_face = U"😢"_u32c;
inline constexpr utf32_char loudly_crying_face = U"😭"_u32c;
inline constexpr utf32_char angry_face = U"😠"_u32c;
inline constexpr utf32_char pouting_face = U"😡"_u32c;
inline constexpr utf32_char face_with_symbols_on_mouth = U"🤬"_u32c;
inline constexpr utf32_char anxious_face_with_sweat = U"😰"_u32c;
inline constexpr utf32_char face_screaming_in_fear = U"😱"_u32c;
inline constexpr utf32_char astonished_face = U"😲"_u32c;
inline constexpr utf32_char sleeping_face = U"😴"_u32c;
inline constexpr utf32_char yawning_face = U"🥱"_u32c;
inline constexpr utf32_char nauseated_face = U"🤢"_u32c;
inline constexpr utf32_char face_vomiting = U"🤮"_u32c;
inline constexpr utf32_char face_with_medical_mask = U"😷"_u32c;
inline constexpr utf32_char hot_face = U"🥵"_u32c;
inline constexpr utf32_char cold_face = U"🥶"_u32c;
inline constexpr utf32_char clown_face = U"🤡"_u32c;
inline constexpr utf32_char cowboy_hat_face = U"🤠"_u32c;
inline constexpr utf32_char partying_face = U"🥳"_u32c;
inline constexpr utf32_char nerd_face = U"🤓"_u32c;
inline constexpr utf32_char skull = U"💀"_u32c;
inline constexpr utf32_char ghost = U"👻"_u32c;
inline constexpr utf32_char alien = U"👽"_u32c;
inline constexpr utf32_char robot = U"🤖"_u32c;
inline constexpr utf32_char pile_of_poo = U"💩"_u32c;
inline constexpr utf32_char waving_hand = U"👋"_u32c;
inline constexpr utf32_char raised_back_of_hand = U"🤚"_u32c;
inline constexpr utf32_char raised_hand = U"✋"_u32c;
inline constexpr utf32_char vulcan_salute = U"🖖"_u32c;
inline constexpr utf32_char ok_hand = U"👌"_u32c;
inline constexpr utf32_char pinched_fingers = U"🤌"_u32c;
inline constexpr utf32_char pinching_hand = U"🤏"_u32c;
inline constexpr utf32_char victory_hand = U"✌"_u32c;
inline constexpr utf32_char crossed_fingers = U"🤞"_u32c;
inline constexpr utf32_char love_you_gesture = U"🤟"_u32c;
inline constexpr utf32_char call_me_hand = U"🤙"_u32c;
inline constexpr utf32_char backhand_index_pointing_up = U"👆"_u32c;
inline constexpr utf32_char thumbs_up = U"👍"_u32c;
inline constexpr utf32_char thumbs_down = U"👎"_u32c;
inline constexpr utf32_char clapping_hands = U"👏"_u32c;
inline constexpr utf32_char raising_hands = U"🙌"_u32c;
inline constexpr utf32_char folded_hands = U"🙏"_u32c;
inline constexpr utf32_char flexed_biceps = U"💪"_u32c;
inline constexpr utf32_char mechanical_arm = U"🦾"_u32c;
inline constexpr utf32_char handshake = U"🤝"_u32c;
inline constexpr utf32_char red_heart = U"❤"_u32c;
inline constexpr utf32_char orange_heart = U"🧡"_u32c;
inline constexpr utf32_char yellow_heart = U"💛"_u32c;
inline constexpr utf32_char green_heart = U"💚"_u32c;
inline constexpr utf32_char blue_heart = U"💙"_u32c;
inline constexpr utf32_char purple_heart = U"💜"_u32c;
inline constexpr utf32_char black_heart = U"🖤"_u32c;
inline constexpr utf32_char white_heart = U"🤍"_u32c;
inline constexpr utf32_char brown_heart = U"🤎"_u32c;
inline constexpr utf32_char broken_heart = U"💔"_u32c;
inline constexpr utf32_char sparkles = U"✨"_u32c;
inline constexpr utf32_char fire = U"🔥"_u32c;
inline constexpr utf32_char collision = U"💥"_u32c;
inline constexpr utf32_char hundred_points = U"💯"_u32c;
inline constexpr utf32_char party_popper = U"🎉"_u32c;
inline constexpr utf32_char wrapped_gift = U"🎁"_u32c;
inline constexpr utf32_char balloon = U"🎈"_u32c;
inline constexpr utf32_char dog_face = U"🐶"_u32c;
inline constexpr utf32_char cat_face = U"🐱"_u32c;
inline constexpr utf32_char mouse_face = U"🐭"_u32c;
inline constexpr utf32_char rabbit_face = U"🐰"_u32c;
inline constexpr utf32_char fox = U"🦊"_u32c;
inline constexpr utf32_char bear = U"🐻"_u32c;
inline constexpr utf32_char panda = U"🐼"_u32c;
inline constexpr utf32_char koala = U"🐨"_u32c;
inline constexpr utf32_char tiger_face = U"🐯"_u32c;
inline constexpr utf32_char lion = U"🦁"_u32c;
inline constexpr utf32_char cow_face = U"🐮"_u32c;
inline constexpr utf32_char pig_face = U"🐷"_u32c;
inline constexpr utf32_char frog = U"🐸"_u32c;
inline constexpr utf32_char monkey_face = U"🐵"_u32c;
inline constexpr utf32_char chicken = U"🐔"_u32c;
inline constexpr utf32_char penguin = U"🐧"_u32c;
inline constexpr utf32_char bird = U"🐦"_u32c;
inline constexpr utf32_char owl = U"🦉"_u32c;
inline constexpr utf32_char unicorn = U"🦄"_u32c;
inline constexpr utf32_char honeybee = U"🐝"_u32c;
inline constexpr utf32_char butterfly = U"🦋"_u32c;
inline constexpr utf32_char lady_beetle = U"🐞"_u32c;
inline constexpr utf32_char turtle = U"🐢"_u32c;
inline constexpr utf32_char snake = U"🐍"_u32c;
inline constexpr utf32_char dragon_face = U"🐲"_u32c;
inline constexpr utf32_char cactus = U"🌵"_u32c;
inline constexpr utf32_char four_leaf_clover = U"🍀"_u32c;
inline constexpr utf32_char maple_leaf = U"🍁"_u32c;
inline constexpr utf32_char leaf_fluttering_in_wind = U"🍃"_u32c;
inline constexpr utf32_char rose = U"🌹"_u32c;
inline constexpr utf32_char sunflower = U"🌻"_u32c;
inline constexpr utf32_char mushroom = U"🍄"_u32c;
inline constexpr utf32_char globe_showing_europe_africa = U"🌍"_u32c;
inline constexpr utf32_char sun = U"☀"_u32c;
inline constexpr utf32_char cloud = U"☁"_u32c;
inline constexpr utf32_char rainbow = U"🌈"_u32c;
inline constexpr utf32_char snowflake = U"❄"_u32c;
inline constexpr utf32_char red_apple = U"🍎"_u32c;
inline constexpr utf32_char green_apple = U"🍏"_u32c;
inline constexpr utf32_char pear = U"🍐"_u32c;
inline constexpr utf32_char peach = U"🍑"_u32c;
inline constexpr utf32_char cherries = U"🍒"_u32c;
inline constexpr utf32_char strawberry = U"🍓"_u32c;
inline constexpr utf32_char blueberries = U"🫐"_u32c;
inline constexpr utf32_char kiwi_fruit = U"🥝"_u32c;
inline constexpr utf32_char watermelon = U"🍉"_u32c;
inline constexpr utf32_char lemon = U"🍋"_u32c;
inline constexpr utf32_char banana = U"🍌"_u32c;
inline constexpr utf32_char pineapple = U"🍍"_u32c;
inline constexpr utf32_char mango = U"🥭"_u32c;
inline constexpr utf32_char grapes = U"🍇"_u32c;
inline constexpr utf32_char avocado = U"🥑"_u32c;
inline constexpr utf32_char carrot = U"🥕"_u32c;
inline constexpr utf32_char hot_pepper = U"🌶"_u32c;
inline constexpr utf32_char ear_of_corn = U"🌽"_u32c;
inline constexpr utf32_char croissant = U"🥐"_u32c;
inline constexpr utf32_char bread = U"🍞"_u32c;
inline constexpr utf32_char pretzel = U"🥨"_u32c;
inline constexpr utf32_char cheese_wedge = U"🧀"_u32c;
inline constexpr utf32_char hamburger = U"🍔"_u32c;
inline constexpr utf32_char pizza = U"🍕"_u32c;
inline constexpr utf32_char french_fries = U"🍟"_u32c;
inline constexpr utf32_char taco = U"🌮"_u32c;
inline constexpr utf32_char burrito = U"🌯"_u32c;
inline constexpr utf32_char sushi = U"🍣"_u32c;
inline constexpr utf32_char steaming_bowl = U"🍜"_u32c;
inline constexpr utf32_char birthday_cake = U"🎂"_u32c;
inline constexpr utf32_char doughnut = U"🍩"_u32c;
inline constexpr utf32_char cookie = U"🍪"_u32c;
inline constexpr utf32_char soft_ice_cream = U"🍦"_u32c;
inline constexpr utf32_char hot_beverage = U"☕"_u32c;
inline constexpr utf32_char beer_mug = U"🍺"_u32c;
inline constexpr utf32_char clinking_beer_mugs = U"🍻"_u32c;
inline constexpr utf32_char wine_glass = U"🍷"_u32c;
inline constexpr utf32_char cocktail_glass = U"🍸"_u32c;
inline constexpr utf32_char tropical_drink = U"🍹"_u32c;
inline constexpr utf32_char soccer_ball = U"⚽"_u32c;
inline constexpr utf32_char basketball = U"🏀"_u32c;
inline constexpr utf32_char volleyball = U"🏐"_u32c;
inline constexpr utf32_char baseball = U"⚾"_u32c;
inline constexpr utf32_char tennis = U"🎾"_u32c;
inline constexpr utf32_char trophy = U"🏆"_u32c;
inline constexpr utf32_char first_place_medal = U"🥇"_u32c;
inline constexpr utf32_char artist_palette = U"🎨"_u32c;
inline constexpr utf32_char musical_note = U"🎵"_u32c;
inline constexpr utf32_char headphones = U"🎧"_u32c;
inline constexpr utf32_char studio_microphone = U"🎙"_u32c;
inline constexpr utf32_char movie_camera = U"🎥"_u32c;
inline constexpr utf32_char camera = U"📷"_u32c;
inline constexpr utf32_char video_camera = U"📹"_u32c;
inline constexpr utf32_char clapper_board = U"🎬"_u32c;
inline constexpr utf32_char light_bulb = U"💡"_u32c;
inline constexpr utf32_char flashlight = U"🔦"_u32c;
inline constexpr utf32_char magnifying_glass_tilted_left = U"🔍"_u32c;
inline constexpr utf32_char link = U"🔗"_u32c;
inline constexpr utf32_char paperclip = U"📎"_u32c;
inline constexpr utf32_char pushpin = U"📌"_u32c;
inline constexpr utf32_char lock = U"🔒"_u32c;
inline constexpr utf32_char key = U"🔑"_u32c;
inline constexpr utf32_char hammer = U"🔨"_u32c;
inline constexpr utf32_char wrench = U"🔧"_u32c;
inline constexpr utf32_char gear = U"⚙"_u32c;
inline constexpr utf32_char laptop = U"💻"_u32c;
inline constexpr utf32_char mobile_phone = U"📱"_u32c;
inline constexpr utf32_char package = U"📦"_u32c;
inline constexpr utf32_char shopping_cart = U"🛒"_u32c;
inline constexpr utf32_char money_bag = U"💰"_u32c;
inline constexpr utf32_char credit_card = U"💳"_u32c;
inline constexpr utf32_char gem_stone = U"💎"_u32c;
inline constexpr utf32_char automobile = U"🚗"_u32c;
inline constexpr utf32_char taxi = U"🚕"_u32c;
inline constexpr utf32_char bus = U"🚌"_u32c;
inline constexpr utf32_char police_car = U"🚓"_u32c;
inline constexpr utf32_char racing_car = U"🏎"_u32c;
inline constexpr utf32_char motorcycle = U"🏍"_u32c;
inline constexpr utf32_char bicycle = U"🚲"_u32c;
inline constexpr utf32_char kick_scooter = U"🛴"_u32c;
inline constexpr utf32_char airplane = U"✈"_u32c;
inline constexpr utf32_char rocket = U"🚀"_u32c;
inline constexpr utf32_char helicopter = U"🚁"_u32c;
inline constexpr utf32_char sailboat = U"⛵"_u32c;
inline constexpr utf32_char anchor = U"⚓"_u32c;
inline constexpr utf32_char fuel_pump = U"⛽"_u32c;
inline constexpr utf32_char house = U"🏠"_u32c;
inline constexpr utf32_char office_building = U"🏢"_u32c;
inline constexpr utf32_char hospital = U"🏥"_u32c;
inline constexpr utf32_char school = U"🏫"_u32c;
inline constexpr utf32_char bank = U"🏦"_u32c;
inline constexpr utf32_char hotel = U"🏨"_u32c;
inline constexpr utf32_char stadium = U"🏟"_u32c;
inline constexpr utf32_char mountain = U"⛰"_u32c;
inline constexpr utf32_char volcano = U"🌋"_u32c;
inline constexpr utf32_char beach_with_umbrella = U"🏖"_u32c;
inline constexpr utf32_char tent = U"⛺"_u32c;
}
}
}

#endif // UTF8_RANGES_CHARACTERS_HPP
