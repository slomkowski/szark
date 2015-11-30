package eu.slomkowski.szark.client.status;

import com.google.gson.annotations.Expose;
import com.google.gson.annotations.SerializedName;

public abstract class AbstractMotor {

    protected byte speedLimit;

    public synchronized byte getSpeedLimit() {
        return speedLimit;
    }

    public abstract void setSpeedLimit(byte speed);

    public void setSpeedLimit(int speed) {
        setSpeedLimit((byte) speed);
    }

    public abstract void stop();

    public abstract class AbstractParams {

        protected byte speedLimit = 0;
        @Expose
        @SerializedName("dir")
        private Direction direction;
        @Expose
        @SerializedName("speed")
        private byte speed;

        AbstractParams() {
            stop();
            speed = 0; // initial speed
        }

        AbstractParams(Direction dir, byte sp) {
            direction = dir;
            speed = sp;
        }

        @Override
        public boolean equals(Object other) {
            if (other == null) {
                return false;
            }
            if (this == other) {
                return true;
            }

            if (other instanceof AbstractParams) {
                final AbstractParams o = (AbstractParams) other;

                return (direction == o.direction) && (speed == o.speed);
            } else {
                return false;
            }
        }

        public synchronized Direction getDirection() {
            return direction;
        }

        public synchronized void setDirection(Direction dir) {
            direction = dir;
        }

        public synchronized byte getSpeed() {
            return speed;
        }

        public synchronized void setSpeed(byte sp) {
            if (sp > speedLimit) {
                speed = speedLimit;
            } else {
                speed = sp;
            }
        }

        public void setSpeed(int speed) {
            setSpeed((byte) speed);
        }

        public byte getSpeedLimit() {
            return speedLimit;
        }

        public void setSpeedLimit(byte speed) {
            if (speed < 0) {
                speedLimit = 0;
            } else {
                speedLimit = speed;
            }
        }

        public void setSpeedLimit(int speed) {
            setSpeedLimit((byte) speed);
        }

        public synchronized void stop() {
            speed = 0;
            direction = Direction.STOP;
        }

        @Override
        public String toString() {
            return "direction: " + direction + ", speed: " + speed;
        }
    }
}
