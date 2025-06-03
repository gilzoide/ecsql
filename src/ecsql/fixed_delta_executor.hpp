namespace ecsql {

class fixed_delta_executor {
public:
	template<typename Fn>
	float execute(float delta_time, float fixed_delta_time, Fn&& f) {
		// Trick to reduce jitter
		// Reference: https://github.com/zmanuel/godot/commit/90a11efc0b27c1128d8203a9be8562fcde006867
		int iterations = 0;
		time_accumulator += delta_time;
		while (time_accumulator + (target_iterations - iterations) * delta_time * 0.5 > delta_time) {
			f();
			time_accumulator -= fixed_delta_time;
			iterations++;
		}
		if (iterations <= target_iterations) {
			target_iterations = iterations;
		}
		else {
			target_iterations = iterations - 1;
		}

		return time_accumulator / fixed_delta_time;
	}

private:
	int target_iterations = 0;
	float time_accumulator = 0;
};

}
