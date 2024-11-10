enum class ExceptionType {
  /** Invalid exception type.*/
  INVALID = 0,
  /** Value out of range. */
  OUT_OF_RANGE = 1,
  /** Conversion/casting error. */
  CONVERSION = 2,
  /** Unknown type in the type subsystem. */
  UNKNOWN_TYPE = 3,
  /** Decimal-related errors. */
  DECIMAL = 4,
  /** Type mismatch. */
  MISMATCH_TYPE = 5,
  /** Division by 0. */
  DIVIDE_BY_ZERO = 6,
  /** Incompatible type. */
  INCOMPATIBLE_TYPE = 8,
  /** Out of memory error */
  OUT_OF_MEMORY = 9,
  /** Method not implemented. */
  NOT_IMPLEMENTED = 11,
  /** Execution exception. */
  EXECUTION = 12,
};