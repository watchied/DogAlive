#ifndef SLIME_KING_CONFIG_H
#define SLIME_KING_CONFIG_H
// Distances and speeds use logical game pixels; times are seconds.
#define KING_MAX_HP 300
#define KING_PHASE_TWO_RATIO 0.5f
#define KING_MOVE_SPEED 12.0f
#define KING_ATTACK_REST 2.0f
#define KING_PHASE_TWO_REST 0.85f
#define KING_INTRO_TIME 1.6f // opening1: total duration, 8 frames at 200ms.
#define KING_PHASE_TRANSITION_TIME 2.8f // opening2: total duration, 14 frames at 200ms.
#define KING_DEATH_TIME 2.0f // Total death duration, 10 frames at 200ms; then become NPC.
#define KING_WALL 4.0f
#define KING_BODY_WIDTH 32.0f
#define KING_BODY_HEIGHT 28.0f
#define KING_BODY_OFFSET_X 8.0f
#define KING_BODY_OFFSET_Y 18.0f
#define KING_SHOT_CAPACITY 64
#define KING_SHOT_LIFETIME 30.0f
#define KING_BUBBLE_DAMAGE 18
#define KING_BUBBLE_SPEED 75.0f
#define KING_BUBBLE_RADIUS 7.0f
#define KING_BUBBLE_BOUNCES 8
#define KING_BUBBLE_WINDUP 0.8f
#define KING_DASH_DAMAGE 24
#define KING_DASH_SPEED 250.0f
#define KING_DASH_WINDUP 0.75f
#define KING_DASH_TIME 0.7f
#define KING_SLAM_DAMAGE 25
#define KING_SLAM_RADIUS 25.0f
#define KING_SLAM_MIN 2
#define KING_SLAM_MAX 5
#define KING_BURROW_TIME 0.8f
#define KING_SLAM_WARNING 0.62f
#define KING_SLAM_FALL_TIME 0.23f
#define KING_SLAM_HEIGHT 100.0f
#define KING_SLAM_RECOVERY 0.35f
#define KING_LASER_DAMAGE 12
#define KING_LASER_SPEED 115.0f
#define KING_LASER_RADIUS 3.0f
#define KING_LASER_BOUNCES 2
#define KING_RADIAL_COUNT 8
#define KING_RADIAL_WINDUP 0.7f
#define KING_SPIRAL_COUNT 10
#define KING_SPIRAL_INTERVAL 0.12f
#define KING_SPIRAL_WINDUP 0.7f
#define KING_SPIRAL_BOUNCES 2
#define KING_BEAM_CHARGE_TIME 2.0f
#define KING_BEAM_DAMAGE 45
#define KING_BEAM_SPEED 1000.0f
#define KING_BEAM_RADIUS 7.0f
#define KING_BEAM_LIFETIME 1.0f
#define KING_BEAM_RECOVERY 0.45f
// Cumulative direct-hit damage during this charge (burn ticks do not interrupt).
#define KING_BEAM_INTERRUPT_DAMAGE 1
#define KING_BEAM_INTERRUPT_STUN 1.0f
#if KING_MAX_HP <= 0 || KING_SLAM_MIN < 1 || KING_SLAM_MAX < KING_SLAM_MIN || KING_RADIAL_COUNT < 1 || KING_SPIRAL_COUNT < 1
#error Invalid Slime King configuration
#endif
#endif
