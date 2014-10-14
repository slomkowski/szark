package eu.slomkowski.szark.client.joystick;

/**
 * Class to store an axis description.
 */
class Axis {

	/**
	 *
	 * @param name
	 *             String name of specific axis, taken from
	 *             Component.getName().
	 * @param moveFactor
	 *             Scaling factor of the value read from the axis.
	 *             Negative values invert the axis.
	 */
	public Axis(String name, float moveFactor) {
		this.name = name;
		this.moveFactor = moveFactor;
	}

	private final String name;
	private final float moveFactor;

	public float getMoveFactor() {
		if (moveFactor < -1.0f) {
			return -1.0f;
		} else if (moveFactor > 1.0f) {
			return 1.0f;
		} else {
			return moveFactor;
		}
	}

	public String getName() {
		return name;
	}
}