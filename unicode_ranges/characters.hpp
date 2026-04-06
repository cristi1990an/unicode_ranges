#ifndef UTF8_RANGES_CHARACTERS_HPP
#define UTF8_RANGES_CHARACTERS_HPP

#include "utf8_char.hpp"
#include "utf16_char.hpp"
#include "utf32_char.hpp"

#define UTF8_RANGES_PUNCTUATION_CHARACTERS(X) \
	X(bullet, "•", u"•") \
	X(middle_dot, "·", u"·") \
	X(ellipsis, "…", u"…") \
	X(en_dash, "–", u"–") \
	X(em_dash, "—", u"—") \
	X(left_single_quotation_mark, "‘", u"‘") \
	X(right_single_quotation_mark, "’", u"’") \
	X(left_double_quotation_mark, "“", u"“") \
	X(right_double_quotation_mark, "”", u"”") \
	X(inverted_exclamation_mark, "¡", u"¡") \
	X(inverted_question_mark, "¿", u"¿")

#define UTF8_RANGES_SYMBOL_CHARACTERS(X) \
	X(copyright_sign, "©", u"©") \
	X(registered_sign, "®", u"®") \
	X(trade_mark_sign, "™", u"™") \
	X(section_sign, "§", u"§") \
	X(paragraph_sign, "¶", u"¶") \
	X(numero_sign, "№", u"№") \
	X(check_mark, "✓", u"✓") \
	X(cross_mark, "✗", u"✗") \
	X(warning_sign, "⚠", u"⚠")

#define UTF8_RANGES_CURRENCY_CHARACTERS(X) \
	X(cent_sign, "¢", u"¢") \
	X(pound_sign, "£", u"£") \
	X(euro_sign, "€", u"€") \
	X(yen_sign, "¥", u"¥") \
	X(bitcoin_sign, "₿", u"₿")

#define UTF8_RANGES_MATH_CHARACTERS(X) \
	X(degree_sign, "°", u"°") \
	X(plus_minus_sign, "±", u"±") \
	X(multiplication_sign, "×", u"×") \
	X(division_sign, "÷", u"÷") \
	X(approximately_equal_to, "≈", u"≈") \
	X(not_equal_to, "≠", u"≠") \
	X(less_than_or_equal_to, "≤", u"≤") \
	X(greater_than_or_equal_to, "≥", u"≥") \
	X(infinity, "∞", u"∞")

#define UTF8_RANGES_ARROW_CHARACTERS(X) \
	X(left_arrow, "←", u"←") \
	X(up_arrow, "↑", u"↑") \
	X(right_arrow, "→", u"→") \
	X(down_arrow, "↓", u"↓") \
	X(left_right_arrow, "↔", u"↔") \
	X(up_down_arrow, "↕", u"↕")

#define UTF8_RANGES_EMOJI_CHARACTERS(X) \
	X(grinning_face, "😀", u"😀") \
	X(beaming_face_with_smiling_eyes, "😁", u"😁") \
	X(face_with_tears_of_joy, "😂", u"😂") \
	X(rolling_on_the_floor_laughing, "🤣", u"🤣") \
	X(grinning_face_with_big_eyes, "😃", u"😃") \
	X(grinning_face_with_smiling_eyes, "😄", u"😄") \
	X(grinning_face_with_sweat, "😅", u"😅") \
	X(smiling_face_with_open_hands, "🤗", u"🤗") \
	X(slightly_smiling_face, "🙂", u"🙂") \
	X(upside_down_face, "🙃", u"🙃") \
	X(winking_face, "😉", u"😉") \
	X(smiling_face_with_smiling_eyes, "😊", u"😊") \
	X(smiling_face_with_halo, "😇", u"😇") \
	X(smiling_face_with_hearts, "🥰", u"🥰") \
	X(smiling_face_with_heart_eyes, "😍", u"😍") \
	X(star_struck, "🤩", u"🤩") \
	X(face_blowing_a_kiss, "😘", u"😘") \
	X(thinking_face, "🤔", u"🤔") \
	X(saluting_face, "🫡", u"🫡") \
	X(zipper_mouth_face, "🤐", u"🤐") \
	X(shushing_face, "🤫", u"🤫") \
	X(neutral_face, "😐", u"😐") \
	X(expressionless_face, "😑", u"😑") \
	X(smirking_face, "😏", u"😏") \
	X(face_with_raised_eyebrow, "🤨", u"🤨") \
	X(pensive_face, "😔", u"😔") \
	X(crying_face, "😢", u"😢") \
	X(loudly_crying_face, "😭", u"😭") \
	X(angry_face, "😠", u"😠") \
	X(pouting_face, "😡", u"😡") \
	X(face_with_symbols_on_mouth, "🤬", u"🤬") \
	X(anxious_face_with_sweat, "😰", u"😰") \
	X(face_screaming_in_fear, "😱", u"😱") \
	X(astonished_face, "😲", u"😲") \
	X(sleeping_face, "😴", u"😴") \
	X(yawning_face, "🥱", u"🥱") \
	X(nauseated_face, "🤢", u"🤢") \
	X(face_vomiting, "🤮", u"🤮") \
	X(face_with_medical_mask, "😷", u"😷") \
	X(hot_face, "🥵", u"🥵") \
	X(cold_face, "🥶", u"🥶") \
	X(clown_face, "🤡", u"🤡") \
	X(cowboy_hat_face, "🤠", u"🤠") \
	X(partying_face, "🥳", u"🥳") \
	X(nerd_face, "🤓", u"🤓") \
	X(skull, "💀", u"💀") \
	X(ghost, "👻", u"👻") \
	X(alien, "👽", u"👽") \
	X(robot, "🤖", u"🤖") \
	X(pile_of_poo, "💩", u"💩") \
	X(waving_hand, "👋", u"👋") \
	X(raised_back_of_hand, "🤚", u"🤚") \
	X(raised_hand, "✋", u"✋") \
	X(vulcan_salute, "🖖", u"🖖") \
	X(ok_hand, "👌", u"👌") \
	X(pinched_fingers, "🤌", u"🤌") \
	X(pinching_hand, "🤏", u"🤏") \
	X(victory_hand, "✌", u"✌") \
	X(crossed_fingers, "🤞", u"🤞") \
	X(love_you_gesture, "🤟", u"🤟") \
	X(call_me_hand, "🤙", u"🤙") \
	X(backhand_index_pointing_up, "👆", u"👆") \
	X(thumbs_up, "👍", u"👍") \
	X(thumbs_down, "👎", u"👎") \
	X(clapping_hands, "👏", u"👏") \
	X(raising_hands, "🙌", u"🙌") \
	X(folded_hands, "🙏", u"🙏") \
	X(flexed_biceps, "💪", u"💪") \
	X(mechanical_arm, "🦾", u"🦾") \
	X(handshake, "🤝", u"🤝") \
	X(red_heart, "❤", u"❤") \
	X(orange_heart, "🧡", u"🧡") \
	X(yellow_heart, "💛", u"💛") \
	X(green_heart, "💚", u"💚") \
	X(blue_heart, "💙", u"💙") \
	X(purple_heart, "💜", u"💜") \
	X(black_heart, "🖤", u"🖤") \
	X(white_heart, "🤍", u"🤍") \
	X(brown_heart, "🤎", u"🤎") \
	X(broken_heart, "💔", u"💔") \
	X(sparkles, "✨", u"✨") \
	X(fire, "🔥", u"🔥") \
	X(collision, "💥", u"💥") \
	X(hundred_points, "💯", u"💯") \
	X(party_popper, "🎉", u"🎉") \
	X(wrapped_gift, "🎁", u"🎁") \
	X(balloon, "🎈", u"🎈") \
	X(dog_face, "🐶", u"🐶") \
	X(cat_face, "🐱", u"🐱") \
	X(mouse_face, "🐭", u"🐭") \
	X(rabbit_face, "🐰", u"🐰") \
	X(fox, "🦊", u"🦊") \
	X(bear, "🐻", u"🐻") \
	X(panda, "🐼", u"🐼") \
	X(koala, "🐨", u"🐨") \
	X(tiger_face, "🐯", u"🐯") \
	X(lion, "🦁", u"🦁") \
	X(cow_face, "🐮", u"🐮") \
	X(pig_face, "🐷", u"🐷") \
	X(frog, "🐸", u"🐸") \
	X(monkey_face, "🐵", u"🐵") \
	X(chicken, "🐔", u"🐔") \
	X(penguin, "🐧", u"🐧") \
	X(bird, "🐦", u"🐦") \
	X(owl, "🦉", u"🦉") \
	X(unicorn, "🦄", u"🦄") \
	X(honeybee, "🐝", u"🐝") \
	X(butterfly, "🦋", u"🦋") \
	X(lady_beetle, "🐞", u"🐞") \
	X(turtle, "🐢", u"🐢") \
	X(snake, "🐍", u"🐍") \
	X(dragon_face, "🐲", u"🐲") \
	X(cactus, "🌵", u"🌵") \
	X(four_leaf_clover, "🍀", u"🍀") \
	X(maple_leaf, "🍁", u"🍁") \
	X(leaf_fluttering_in_wind, "🍃", u"🍃") \
	X(rose, "🌹", u"🌹") \
	X(sunflower, "🌻", u"🌻") \
	X(mushroom, "🍄", u"🍄") \
	X(globe_showing_europe_africa, "🌍", u"🌍") \
	X(sun, "☀", u"☀") \
	X(cloud, "☁", u"☁") \
	X(rainbow, "🌈", u"🌈") \
	X(snowflake, "❄", u"❄") \
	X(red_apple, "🍎", u"🍎") \
	X(green_apple, "🍏", u"🍏") \
	X(pear, "🍐", u"🍐") \
	X(peach, "🍑", u"🍑") \
	X(cherries, "🍒", u"🍒") \
	X(strawberry, "🍓", u"🍓") \
	X(blueberries, "🫐", u"🫐") \
	X(kiwi_fruit, "🥝", u"🥝") \
	X(watermelon, "🍉", u"🍉") \
	X(lemon, "🍋", u"🍋") \
	X(banana, "🍌", u"🍌") \
	X(pineapple, "🍍", u"🍍") \
	X(mango, "🥭", u"🥭") \
	X(grapes, "🍇", u"🍇") \
	X(avocado, "🥑", u"🥑") \
	X(carrot, "🥕", u"🥕") \
	X(hot_pepper, "🌶", u"🌶") \
	X(ear_of_corn, "🌽", u"🌽") \
	X(croissant, "🥐", u"🥐") \
	X(bread, "🍞", u"🍞") \
	X(pretzel, "🥨", u"🥨") \
	X(cheese_wedge, "🧀", u"🧀") \
	X(hamburger, "🍔", u"🍔") \
	X(pizza, "🍕", u"🍕") \
	X(french_fries, "🍟", u"🍟") \
	X(taco, "🌮", u"🌮") \
	X(burrito, "🌯", u"🌯") \
	X(sushi, "🍣", u"🍣") \
	X(steaming_bowl, "🍜", u"🍜") \
	X(birthday_cake, "🎂", u"🎂") \
	X(doughnut, "🍩", u"🍩") \
	X(cookie, "🍪", u"🍪") \
	X(soft_ice_cream, "🍦", u"🍦") \
	X(hot_beverage, "☕", u"☕") \
	X(beer_mug, "🍺", u"🍺") \
	X(clinking_beer_mugs, "🍻", u"🍻") \
	X(wine_glass, "🍷", u"🍷") \
	X(cocktail_glass, "🍸", u"🍸") \
	X(tropical_drink, "🍹", u"🍹") \
	X(soccer_ball, "⚽", u"⚽") \
	X(basketball, "🏀", u"🏀") \
	X(volleyball, "🏐", u"🏐") \
	X(baseball, "⚾", u"⚾") \
	X(tennis, "🎾", u"🎾") \
	X(trophy, "🏆", u"🏆") \
	X(first_place_medal, "🥇", u"🥇") \
	X(artist_palette, "🎨", u"🎨") \
	X(musical_note, "🎵", u"🎵") \
	X(headphones, "🎧", u"🎧") \
	X(studio_microphone, "🎙", u"🎙") \
	X(movie_camera, "🎥", u"🎥") \
	X(camera, "📷", u"📷") \
	X(video_camera, "📹", u"📹") \
	X(clapper_board, "🎬", u"🎬") \
	X(light_bulb, "💡", u"💡") \
	X(flashlight, "🔦", u"🔦") \
	X(magnifying_glass_tilted_left, "🔍", u"🔍") \
	X(link, "🔗", u"🔗") \
	X(paperclip, "📎", u"📎") \
	X(pushpin, "📌", u"📌") \
	X(lock, "🔒", u"🔒") \
	X(key, "🔑", u"🔑") \
	X(hammer, "🔨", u"🔨") \
	X(wrench, "🔧", u"🔧") \
	X(gear, "⚙", u"⚙") \
	X(laptop, "💻", u"💻") \
	X(mobile_phone, "📱", u"📱") \
	X(package, "📦", u"📦") \
	X(shopping_cart, "🛒", u"🛒") \
	X(money_bag, "💰", u"💰") \
	X(credit_card, "💳", u"💳") \
	X(gem_stone, "💎", u"💎") \
	X(automobile, "🚗", u"🚗") \
	X(taxi, "🚕", u"🚕") \
	X(bus, "🚌", u"🚌") \
	X(police_car, "🚓", u"🚓") \
	X(racing_car, "🏎", u"🏎") \
	X(motorcycle, "🏍", u"🏍") \
	X(bicycle, "🚲", u"🚲") \
	X(kick_scooter, "🛴", u"🛴") \
	X(airplane, "✈", u"✈") \
	X(rocket, "🚀", u"🚀") \
	X(helicopter, "🚁", u"🚁") \
	X(sailboat, "⛵", u"⛵") \
	X(anchor, "⚓", u"⚓") \
	X(fuel_pump, "⛽", u"⛽") \
	X(house, "🏠", u"🏠") \
	X(office_building, "🏢", u"🏢") \
	X(hospital, "🏥", u"🏥") \
	X(school, "🏫", u"🏫") \
	X(bank, "🏦", u"🏦") \
	X(hotel, "🏨", u"🏨") \
	X(stadium, "🏟", u"🏟") \
	X(mountain, "⛰", u"⛰") \
	X(volcano, "🌋", u"🌋") \
	X(beach_with_umbrella, "🏖", u"🏖") \
	X(tent, "⛺", u"⛺")

#define UTF8_RANGES_DEFINE_UTF8_CHARACTER(name, utf8_literal, utf16_literal) inline constexpr auto name = utf8_literal##_u8c;
#define UTF8_RANGES_DEFINE_UTF16_CHARACTER(name, utf8_literal, utf16_literal) inline constexpr auto name = static_cast<utf16_char>(utf8_literal##_u8c);
#define UTF8_RANGES_DEFINE_UTF32_CHARACTER(name, utf8_literal, utf16_literal) inline constexpr auto name = static_cast<utf32_char>(utf8_literal##_u8c);

namespace unicode_ranges::characters
{
using namespace unicode_ranges::literals;

namespace utf8
{
namespace punctuation { UTF8_RANGES_PUNCTUATION_CHARACTERS(UTF8_RANGES_DEFINE_UTF8_CHARACTER) }
namespace symbols { UTF8_RANGES_SYMBOL_CHARACTERS(UTF8_RANGES_DEFINE_UTF8_CHARACTER) }
namespace currency { UTF8_RANGES_CURRENCY_CHARACTERS(UTF8_RANGES_DEFINE_UTF8_CHARACTER) }
namespace math { UTF8_RANGES_MATH_CHARACTERS(UTF8_RANGES_DEFINE_UTF8_CHARACTER) }
namespace arrows { UTF8_RANGES_ARROW_CHARACTERS(UTF8_RANGES_DEFINE_UTF8_CHARACTER) }
namespace emojis { UTF8_RANGES_EMOJI_CHARACTERS(UTF8_RANGES_DEFINE_UTF8_CHARACTER) }
}

namespace utf16
{
namespace punctuation { UTF8_RANGES_PUNCTUATION_CHARACTERS(UTF8_RANGES_DEFINE_UTF16_CHARACTER) }
namespace symbols { UTF8_RANGES_SYMBOL_CHARACTERS(UTF8_RANGES_DEFINE_UTF16_CHARACTER) }
namespace currency { UTF8_RANGES_CURRENCY_CHARACTERS(UTF8_RANGES_DEFINE_UTF16_CHARACTER) }
namespace math { UTF8_RANGES_MATH_CHARACTERS(UTF8_RANGES_DEFINE_UTF16_CHARACTER) }
namespace arrows { UTF8_RANGES_ARROW_CHARACTERS(UTF8_RANGES_DEFINE_UTF16_CHARACTER) }
namespace emojis { UTF8_RANGES_EMOJI_CHARACTERS(UTF8_RANGES_DEFINE_UTF16_CHARACTER) }
}

namespace utf32
{
namespace punctuation { UTF8_RANGES_PUNCTUATION_CHARACTERS(UTF8_RANGES_DEFINE_UTF32_CHARACTER) }
namespace symbols { UTF8_RANGES_SYMBOL_CHARACTERS(UTF8_RANGES_DEFINE_UTF32_CHARACTER) }
namespace currency { UTF8_RANGES_CURRENCY_CHARACTERS(UTF8_RANGES_DEFINE_UTF32_CHARACTER) }
namespace math { UTF8_RANGES_MATH_CHARACTERS(UTF8_RANGES_DEFINE_UTF32_CHARACTER) }
namespace arrows { UTF8_RANGES_ARROW_CHARACTERS(UTF8_RANGES_DEFINE_UTF32_CHARACTER) }
namespace emojis { UTF8_RANGES_EMOJI_CHARACTERS(UTF8_RANGES_DEFINE_UTF32_CHARACTER) }
}
}

#undef UTF8_RANGES_DEFINE_UTF32_CHARACTER
#undef UTF8_RANGES_DEFINE_UTF16_CHARACTER
#undef UTF8_RANGES_DEFINE_UTF8_CHARACTER
#undef UTF8_RANGES_EMOJI_CHARACTERS
#undef UTF8_RANGES_ARROW_CHARACTERS
#undef UTF8_RANGES_MATH_CHARACTERS
#undef UTF8_RANGES_CURRENCY_CHARACTERS
#undef UTF8_RANGES_SYMBOL_CHARACTERS
#undef UTF8_RANGES_PUNCTUATION_CHARACTERS

#endif // UTF8_RANGES_CHARACTERS_HPP
