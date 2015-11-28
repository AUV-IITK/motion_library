#include <ros/ros.h>
#include <std_msgs/Float32.h>
#include <actionlib/server/simple_action_server.h>
#include <motionlibrary/ForwardAction.h>

typedef actionlib::SimpleActionServer<motionlibrary::ForwardAction> Server;

class forwardAction{
	private:
		ros::NodeHandle nh_;
		Server forwardServer_;
		std::string action_name_;
		motionlibrary::ForwardFeedback feedback_;
		motionlibrary::ForwardResult result_;
		ros::Subscriber sub_;
		float timeSpent, motionTime;
		bool success;

	public:
		forwardAction(std::string name):
			//here we are defining the server, third argument is optional
	    	forwardServer_(nh_, name, boost::bind(&forwardAction::analysisCB, this, _1), false),
    		action_name_(name)
		{
//			forwardServer_.registerGoalCallback(boost::bind(&forwardAction::goalCB, this));
			forwardServer_.registerPreemptCallback(boost::bind(&forwardAction::preemptCB, this));

//this type callback can be used if we want to do the callback from some specific node
//			sub_ = nh_.subscribe("name of the node", 1, &forwardAction::analysisCB, this);
			forwardServer_.start();
		}

		~forwardAction(void){
		}

//Some strange warning was occuring if we were using this goalCB function. Aprntly there was some other callback function for goals
			// void goalCB(){
			// 	timeSpent = 0;
			// 	motionTime = forwardServer_.acceptNewGoal()->MotionTime;
			// 	ROS_INFO("%s: New goal recieved %f", action_name_.c_str(),motionTime);
			// }

		void preemptCB(void){
			//this command cancels the previous goal
			forwardServer_.setPreempted();
			ROS_INFO("%s: Preempted", action_name_.c_str());
		}

		void analysisCB(const motionlibrary::ForwardGoalConstPtr goal){
			ROS_INFO("Inside analysisCB");
		    ros::Rate looprate(1);
		    success = true;
			if (!forwardServer_.isActive())
				return;

			for(timeSpent=0; timeSpent <= goal->MotionTime; timeSpent++){
				if (forwardServer_.isPreemptRequested() || !ros::ok())
				{
					ROS_INFO("%s: Preempted", action_name_.c_str());
			        // set the action state to preempted
        			forwardServer_.setPreempted();
        			success = false;
        			break;
				}
				// publish the feedback
				feedback_.TimeRemaining = goal->MotionTime - timeSpent;
				forwardServer_.publishFeedback(feedback_);
				ROS_INFO("timeSpent %f", timeSpent);
				looprate.sleep();				
			}
			if(success){
				result_.MotionCompleted = success;
				ROS_INFO("%s: Succeeded", action_name_.c_str());
				// set the action state to succeeded
				forwardServer_.setSucceeded(result_);
			}

		}
};

int main(int argc, char** argv){
	ros::init(argc, argv, "forward");
	ROS_INFO("Waiting for Goal");
	forwardAction forward(ros::this_node::getName());

	ros::spin();
	return 0;
}