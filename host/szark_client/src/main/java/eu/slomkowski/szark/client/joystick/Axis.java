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
	 * @param mFactor
	 *             Scaling factor of the value read from the axis.
	 *             Negative values invert the axis.
	 */
	public Axis(String name, float mFactor) {
		this.name = name;
		this.mFactor = mFactor;
	}

	private final String name;
	private final float mFactor;

	public float getMFactor() {
		if (mFactor < -1.0f) {
			return -1.0f;
		} else if (mFactor > 1.0f) {
			return 1.0f;
		} else {
			return mFactor;
		}
	}

	public String getName() {
		return name;
	}
}