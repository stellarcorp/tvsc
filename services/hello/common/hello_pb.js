goog.module('tvsc.service.hello.Hello');

const PbRequest = goog.require('proto.tvsc.service.hello.HelloRequest');

exports = class {
  /**
   * Creates a new HelloRequest.
   */
  constructor() {
    /**
     * @private
     * @const {!PbRequest}
     */
    this.message_ = new PbRequest();
  }
};
